/*
  Copyright (C) 2017 TNO

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <opm/parser/eclipse/EclipseState/AquiferCT.hpp>

namespace Opm {

    AquiferCT::AquiferCT(const EclipseState& eclState, const Deck& deck)
    { 
        if (!deck.hasKeyword("AQUCT"))
            return;

        const auto& aquctKeyword = deck.getKeyword("AQUCT");

        for (auto& aquctRecord : aquctKeyword){
  
            AquiferCT::AQUCT_data data;

            if( deck.hasKeyword( "FIELD" ) ) {
                data.c1 = 0.006328*1.5797e5; // conversion factor constant C1 to SI
                data.c2 = 1.1191*5.6164; // conversion factor constant C2 to SI
            }
            else if( deck.hasKeyword( "METRIC" )) {
                data.c1 = 0.008527*1.1727e9; // conversion factor constant C1 to SI
                data.c2 = 6.283; // conversion factor constant C2 to SI        
            }
    
            data.aquiferID = aquctRecord.getItem("AQUIFER_ID").template get<int>(0);
            data.h = aquctRecord.getItem("THICKNESS_AQ").getSIDouble(0);
            data.p0 = aquctRecord.getItem("P_INI").getSIDouble(0);
            data.p0_defaulted = aquctRecord.getItem("P_INI").defaultApplied(0);
            data.phi_aq = aquctRecord.getItem("PORO_AQ").getSIDouble(0);
            data.d0 = aquctRecord.getItem("DAT_DEPTH").getSIDouble(0);
            data.C_t = aquctRecord.getItem("C_T").getSIDouble(0);
            data.r_o = aquctRecord.getItem("RAD").getSIDouble(0);
            data.k_a = aquctRecord.getItem("PERM_AQ").getSIDouble(0);
            data.theta = aquctRecord.getItem("INFLUENCE_ANGLE").getSIDouble(0)/360.0; // We are taking the user input and making it into a fraction for the AQUCT parameter
            data.inftableID = aquctRecord.getItem("TABLE_NUM_INFLUENCE_FN").template get<int>(0);
            data.pvttableID = aquctRecord.getItem("TABLE_NUM_WATER_PRESS").template get<int>(0);

            // Get the correct influence table values
            if (data.inftableID > 1){
                const auto& aqutabTable = eclState.getTableManager().getAqutabTables().getTable(data.inftableID - 2);
                const auto& aqutab_tdColumn = aqutabTable.getColumn(0);
                const auto& aqutab_piColumn = aqutabTable.getColumn(1);
                data.td = aqutab_tdColumn.vectorCopy();
                data.pi = aqutab_piColumn.vectorCopy();
                }
            else
                {
                    set_default_tables(data.td,data.pi);
                }
                m_aquct.push_back( std::move(data) );
            }     
    }

    const std::vector<AquiferCT::AQUCT_data>& AquiferCT::getAquifers() const
    {
        return m_aquct;
    }

    int AquiferCT::getAqInflTabID(size_t aquiferIndex)
    {
        return m_aquct.at(aquiferIndex).inftableID;
    }

    int AquiferCT::getAqPvtTabID(size_t aquiferIndex)
    {
        return m_aquct.at(aquiferIndex).pvttableID;
    }

}
