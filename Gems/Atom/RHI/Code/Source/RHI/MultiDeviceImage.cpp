/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Atom/RHI/ImageFrameAttachment.h>
#include <Atom/RHI/SingleDeviceImageView.h>
#include <Atom/RHI/MultiDeviceImage.h>

namespace AZ::RHI
{
    void MultiDeviceImage::SetDescriptor(const ImageDescriptor& descriptor)
    {
        m_descriptor = descriptor;
        m_aspectFlags = GetImageAspectFlags(descriptor.m_format);
    }

    const ImageDescriptor& MultiDeviceImage::GetDescriptor() const
    {
        return m_descriptor;
    }

    void MultiDeviceImage::GetSubresourceLayout(MultiDeviceImageSubresourceLayout& subresourceLayout, ImageAspectFlags aspectFlags) const
    {
        ImageSubresourceRange subresourceRange;
        subresourceRange.m_mipSliceMin = 0;
        subresourceRange.m_mipSliceMax = 0;
        subresourceRange.m_arraySliceMin = 0;
        subresourceRange.m_arraySliceMax = 0;
        subresourceRange.m_aspectFlags = aspectFlags;

        IterateObjects<SingleDeviceImage>([&subresourceRange, &subresourceLayout](auto deviceIndex, auto deviceImage)
        {
            deviceImage->GetSubresourceLayouts(subresourceRange, &subresourceLayout.GetDeviceImageSubresource(deviceIndex), nullptr);
        });
    }

    const ImageFrameAttachment* MultiDeviceImage::GetFrameAttachment() const
    {
        return static_cast<const ImageFrameAttachment*>(MultiDeviceResource::GetFrameAttachment());
    }

    Ptr<MultiDeviceImageView> MultiDeviceImage::BuildImageView(const ImageViewDescriptor& imageViewDescriptor)
    {
        //? TODO: We currently need to cache the SingleDeviceImageViews (as no one else is doing that atm),
        //?       can possibly be removed once everything handles MultiDeviceResources everywhere
        AZStd::unordered_map<int, Ptr<RHI::SingleDeviceImageView>> m_cache;
        IterateObjects<SingleDeviceImage>([&imageViewDescriptor, &m_cache](auto deviceIndex, auto deviceImage)
        {
            m_cache[deviceIndex] = deviceImage->GetImageView(imageViewDescriptor);
        });
        return aznew MultiDeviceImageView{ this, imageViewDescriptor, AZStd::move(m_cache) };
    }

    uint32_t MultiDeviceImage::GetResidentMipLevel() const
    {
        auto minLevel{AZStd::numeric_limits<uint32_t>::max()};
        IterateObjects<SingleDeviceImage>([&minLevel]([[maybe_unused]] auto deviceIndex, auto deviceImage)
        {
            minLevel = AZStd::min(minLevel, deviceImage->GetResidentMipLevel());
        });
        return minLevel;
    }

    bool MultiDeviceImage::IsStreamable() const
    {
        bool isStreamable{true};
        IterateObjects<SingleDeviceImage>([&isStreamable]([[maybe_unused]] auto deviceIndex, auto deviceImage)
        {
            isStreamable &= deviceImage->IsStreamable();
        });
        return isStreamable;
    }

    ImageAspectFlags MultiDeviceImage::GetAspectFlags() const
    {
        return m_aspectFlags;
    }

    const HashValue64 MultiDeviceImage::GetHash() const
    {
        HashValue64 hash = HashValue64{ 0 };
        hash = m_descriptor.GetHash();
        hash = TypeHash64(m_supportedQueueMask, hash);
        hash = TypeHash64(m_aspectFlags, hash);
        return hash;
    }

    void MultiDeviceImage::Shutdown()
    {
        IterateObjects<SingleDeviceImage>([]([[maybe_unused]] auto deviceIndex, auto deviceImage)
        {
            deviceImage->Shutdown();
        });

        MultiDeviceResource::Shutdown();
    }

    void MultiDeviceImage::InvalidateViews()
    {
        IterateObjects<SingleDeviceImage>([]([[maybe_unused]] auto deviceIndex, auto deviceImage)
        {
            deviceImage->InvalidateViews();
        });
    }

    //! Given a device index, return the corresponding SingleDeviceBufferView for the selected device
    const RHI::Ptr<RHI::SingleDeviceImageView> MultiDeviceImageView::GetDeviceImageView(int deviceIndex) const
    {
        AZ_Error(
            "MultiDeviceImageView",
            m_cache.find(deviceIndex) != m_cache.end(),
            "No SingleDeviceImageView found for device index %d\n",
            deviceIndex);

        return m_cache.at(deviceIndex);
    }
} // namespace AZ::RHI
