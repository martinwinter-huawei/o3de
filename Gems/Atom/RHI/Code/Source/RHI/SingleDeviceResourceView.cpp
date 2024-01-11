/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Atom/RHI/SingleDeviceResource.h>
#include <Atom/RHI/SingleDeviceResourceView.h>
#include <iostream>
namespace AZ::RHI
{
    ResultCode SingleDeviceResourceView::Init(const SingleDeviceResource& resource)
    {
        RHI::Device& device = resource.GetDevice();

        m_resource = &resource;
        m_version = resource.GetVersion();
        ResultCode resultCode = InitInternal(device, resource);
        if (resultCode != ResultCode::Success)
        {
            m_resource = nullptr;
            return resultCode;
        }

        DeviceObject::Init(device);
        ResourceInvalidateBus::Handler::BusConnect(&resource);
        return ResultCode::Success;
    }

    void SingleDeviceResourceView::Shutdown()
    {
        if (IsInitialized())
        {
            ResourceInvalidateBus::Handler::BusDisconnect(m_resource.get());
            ShutdownInternal();

            m_resource->EraseResourceView(this);
            m_resource = nullptr;
            DeviceObject::Shutdown();
        }
    }

    const SingleDeviceResource& SingleDeviceResourceView::GetResource() const
    {
        return *m_resource;
    }

    bool SingleDeviceResourceView::IsStale(bool framebuffer) const
    {
        if (framebuffer && m_resource && m_resource->GetVersion() != m_version)
        {
            std::cerr << "This resource is stale  resVer: " << m_resource->GetVersion() << " | viewVer: " << m_version
                      << " | Resource: " << m_resource.get() << " | ResourceView: " << this << std::endl;
            AZ_Printf("SingleDeviceResource", "hello");
        }
        return m_resource && m_resource->GetVersion() != m_version;
    }

    ResultCode SingleDeviceResourceView::OnResourceInvalidate()
    {
        AZ_PROFILE_FUNCTION(RHI);
        ResultCode resultCode = InvalidateInternal();
        if (resultCode == ResultCode::Success)
        {
            std::cerr << "SingleDeviceResourceView::OnResourceInvalidate " << m_resource->GetVersion() << " | " << m_version
                      << " | Resource: " << m_resource.get() << " | ResourceView: " << this << std::endl;
            m_version = m_resource->GetVersion();
        }
        else
        {
            std::cerr << "SingleDeviceResourceView::OnResourceInvalidate not successful" << std::endl;
        }
        return resultCode;
    }
}
