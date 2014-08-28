/* rtmdss
 *
 * Header file for the DDS subscriber port base class.
 *
 * Copyright 2011 Geoffrey Biggs geoffrey.biggs@aist.go.jp
 *     RT-Synthesis Research Group
 *     Intelligent Systems Research Institute,
 *     National Institute of Advanced Industrial Science and Technology (AIST),
 *     Japan
 *     All rights reserved.
 *
 * This file is part of rtmdds.
 *
 * rtmdss is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2.1 of the License,
 * or (at your option) any later version.
 *
 * rtmdss is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with rtmdss. If not, see
 * <http://www.gnu.org/licenses/>.
 */

#if !defined(DDSSUBPORTBASE_H_)
#define DDSSUBPORTBASE_H_

#include <coil/Properties.h>
#include <rtm/PortBase.h>
#include <string>

namespace RTC
{
    class DDSSubPortBase
        : public PortBase
    {
        public:
            DDSSubPortBase(std::string name);
            ~DDSSubPortBase();

            virtual void init(coil::Properties const& props);
            coil::Properties const& properties() const { return props_; }

            void activateInterfaces() { /* Nothing to do */ };
            void deactivateInterfaces() { /* Nothing to do */ };

            bool read();

        protected:
            virtual ReturnCode_t publishInterfaces(ConnectorProfile& cp);
            virtual ReturnCode_t subscribeInterfaces(ConnectorProfile const& cp) = 0;
            virtual void unsubscribeInterfaces(ConnectorProfile const& cp)
                { /* Nothing to do */ };

            coil::Properties props_;
    };
}; // namespace RTC

#endif // !defined(DDSSUBPORTBASE_H_)

