/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Atom/RHI/MultiDeviceDrawItem.h>

#include <Atom/RHI/RHISystemInterface.h>

#include <iostream>

namespace AZ::RHI
{
    MultiDeviceDrawItem::MultiDeviceDrawItem(MultiDevice::DeviceMask deviceMask)
        : m_deviceMask{ deviceMask }
    {
        auto deviceCount{ RHI::RHISystemInterface::Get()->GetDeviceCount() };

        for (int deviceIndex = 0; deviceIndex < deviceCount; ++deviceIndex)
        {
            if ((AZStd::to_underlying(m_deviceMask) >> deviceIndex) & 1)
            {
                m_deviceDrawItems.emplace(deviceIndex, SingleDeviceDrawItem{});
            }
        }

        for(auto& [deviceIndex, drawItem] : m_deviceDrawItems)
        {
            m_deviceDrawItemPtrs.emplace(deviceIndex, &drawItem);
        }

        std::cerr << this << " called the default constructor" << std::endl;
    }

    MultiDeviceDrawItem::MultiDeviceDrawItem(MultiDevice::DeviceMask deviceMask, AZStd::unordered_map<int, SingleDeviceDrawItem*>&& deviceDrawItemPtrs)
        : m_deviceMask{ deviceMask }
        , m_deviceDrawItemPtrs{ deviceDrawItemPtrs }
    {
        std::cerr << this << " called the constructor with a full map" << std::endl;
    }
} // namespace AZ::RHI
