/*
  Copyright 2014 Statoil ASA.

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

#include <stdexcept>
#include <iostream>
#include <memory>

#define BOOST_TEST_MODULE EclipseGridTests

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <opm/parser/eclipse/Parser/Parser.hpp>

#include <opm/parser/eclipse/Deck/Section.hpp>
#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>
#include <opm/parser/eclipse/EclipseState/Eclipse3DProperties.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/Box.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/GridProperties.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/GridProperty.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/TableManager.hpp>

static const Opm::DeckKeyword createSATNUMKeyword( ) {
    const char* deckData =
    "SATNUM \n"
    "  0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 / \n"
    "\n";

    Opm::Parser parser;
    Opm::Deck deck = parser.parseString(deckData);
    return deck.getKeyword("SATNUM");
}

static const Opm::DeckKeyword createTABDIMSKeyword( ) {
    const char* deckData =
    "TABDIMS\n"
    "  0 1 2 3 4 5 / \n"
    "\n";

    Opm::Parser parser;
    Opm::Deck deck = parser.parseString(deckData);
    return deck.getKeyword("TABDIMS");
}

BOOST_AUTO_TEST_CASE(Empty) {
    typedef Opm::GridProperty<int>::SupportedKeywordInfo SupportedKeywordInfo;
    SupportedKeywordInfo keywordInfo("SATNUM" , 77, "1");
    Opm::GridProperty<int> gridProperty( 5 , 5 , 4 , keywordInfo);
    const std::vector<int>& data = gridProperty.getData();
    BOOST_CHECK_EQUAL( 100U , data.size());
    BOOST_CHECK_EQUAL( 100U , gridProperty.getCartesianSize());
    BOOST_CHECK_EQUAL( 5U , gridProperty.getNX());
    BOOST_CHECK_EQUAL( 5U , gridProperty.getNY());
    BOOST_CHECK_EQUAL( 4U , gridProperty.getNZ());
    for (size_t k=0; k < 4; k++) {
        for (size_t j=0; j < 5; j++) {
            for (size_t i=0; i < 5; i++) {
                size_t g = i + j*5 + k*25;
                BOOST_CHECK_EQUAL( 77 , data[g] );
                BOOST_CHECK_EQUAL( 77 , gridProperty.iget( g ));
                BOOST_CHECK_EQUAL( 77 , gridProperty.iget( i,j,k ));
            }
        }
    }
}


BOOST_AUTO_TEST_CASE(HasNAN) {
    double nan = std::numeric_limits<double>::quiet_NaN();
    typedef Opm::GridProperty<double>::SupportedKeywordInfo SupportedKeywordInfo;
    SupportedKeywordInfo keywordInfo("PORO" , nan , "1");
    Opm::GridProperty<double> poro( 2 , 2 , 1 , keywordInfo);

    BOOST_CHECK( poro.containsNaN() );
    poro.iset(0,0.15);
    poro.iset(1,0.15);
    poro.iset(2,0.15);
    BOOST_CHECK( poro.containsNaN() );
    poro.iset(3,0.15);
    BOOST_CHECK( !poro.containsNaN() );
}

BOOST_AUTO_TEST_CASE(EmptyDefault) {
    typedef Opm::GridProperty<int>::SupportedKeywordInfo SupportedKeywordInfo;
    SupportedKeywordInfo keywordInfo("SATNUM" , 0, "1");
    Opm::GridProperty<int> gridProperty( /*nx=*/10,
                                         /*ny=*/10,
                                         /*nz=*/1 ,
                                         keywordInfo);
    const std::vector<int>& data = gridProperty.getData();
    BOOST_CHECK_EQUAL( 100U , data.size());
    for (size_t i=0; i < data.size(); i++)
        BOOST_CHECK_EQUAL( 0 , data[i] );
}

BOOST_AUTO_TEST_CASE(SetFromDeckKeyword_notData_Throws) {
    const auto& tabdimsKw = createTABDIMSKeyword();
    typedef Opm::GridProperty<int>::SupportedKeywordInfo SupportedKeywordInfo;
    SupportedKeywordInfo keywordInfo("TABDIMS" , 100, "1");
    Opm::GridProperty<int> gridProperty( 6 ,1,1 , keywordInfo);
    BOOST_CHECK_THROW( gridProperty.loadFromDeckKeyword( tabdimsKw ) , std::invalid_argument );
}


BOOST_AUTO_TEST_CASE(SetFromDeckKeyword_wrong_size_throws) {
    const auto& satnumKw = createSATNUMKeyword();
    typedef Opm::GridProperty<int>::SupportedKeywordInfo SupportedKeywordInfo;
    SupportedKeywordInfo keywordInfo("SATNUM" , 66, "1");
    Opm::GridProperty<int> gridProperty( 15 ,1,1, keywordInfo);
    BOOST_CHECK_THROW( gridProperty.loadFromDeckKeyword( satnumKw ) , std::invalid_argument );
}



BOOST_AUTO_TEST_CASE(SetFromDeckKeyword) {
    const auto& satnumKw = createSATNUMKeyword();
    typedef Opm::GridProperty<int>::SupportedKeywordInfo SupportedKeywordInfo;
    SupportedKeywordInfo keywordInfo("SATNUM" , 99, "1");
    Opm::GridProperty<int> gridProperty( 4 , 4 , 2 , keywordInfo);
    gridProperty.loadFromDeckKeyword( satnumKw );
    const std::vector<int>& data = gridProperty.getData();
    for (size_t k=0; k < 2; k++) {
        for (size_t j=0; j < 4; j++) {
            for (size_t i=0; i < 4; i++) {
                size_t g = i + j*4 + k*16;

                BOOST_CHECK_EQUAL( g , data[g] );
                BOOST_CHECK_EQUAL( g , gridProperty.iget(g) );
                BOOST_CHECK_EQUAL( g , gridProperty.iget(i,j,k) );

            }
        }
    }
}

BOOST_AUTO_TEST_CASE(copy) {
    typedef Opm::GridProperty<int>::SupportedKeywordInfo SupportedKeywordInfo;
    SupportedKeywordInfo keywordInfo1("P1", 0, "1");
    SupportedKeywordInfo keywordInfo2("P2", 9, "1");
    Opm::GridProperty<int> prop1(4, 4, 2, keywordInfo1);
    Opm::GridProperty<int> prop2(4, 4, 2, keywordInfo2);

    Opm::Box global(4, 4, 2);
    Opm::Box layer0(global, 0, 3, 0, 3, 0, 0);

    prop2.copyFrom(prop1, layer0);

    for (size_t j = 0; j < 4; j++) {
        for (size_t i = 0; i < 4; i++) {

            BOOST_CHECK_EQUAL(prop2.iget(i, j, 0), 0);
            BOOST_CHECK_EQUAL(prop2.iget(i, j, 1), 9);
        }
    }
}


BOOST_AUTO_TEST_CASE(SCALE) {
    typedef Opm::GridProperty<int>::SupportedKeywordInfo SupportedKeywordInfo;
    SupportedKeywordInfo keywordInfo1( "P1", 1, "1" );
    SupportedKeywordInfo keywordInfo2( "P2", 9, "1" );

    Opm::GridProperty<int> prop1( 4, 4, 2, keywordInfo1 );
    Opm::GridProperty<int> prop2( 4, 4, 2, keywordInfo2 );

    Opm::Box global( 4, 4, 2 );
    Opm::Box layer0( global, 0, 3, 0, 3, 0, 0 );

    prop2.copyFrom( prop1, layer0 );
    prop2.scale( 2, global );
    prop2.scale( 2, layer0 );

    for (size_t j = 0; j < 4; j++) {
        for (size_t i = 0; i < 4; i++) {

            BOOST_CHECK_EQUAL( prop2.iget( i, j, 0 ), 4 );
            BOOST_CHECK_EQUAL( prop2.iget( i, j, 1 ), 18 );
        }
    }
}

BOOST_AUTO_TEST_CASE(SET) {
    typedef Opm::GridProperty<int>::SupportedKeywordInfo SupportedKeywordInfo;
    SupportedKeywordInfo keywordInfo( "P1", 1, "1" );
    Opm::GridProperty<int> prop( 4, 4, 2, keywordInfo );

    Opm::Box global( 4, 4, 2 );
    Opm::Box layer0( global, 0, 3, 0, 3, 0, 0 );

    prop.setScalar( 2, global );
    prop.setScalar( 4, layer0 );

    for (size_t j = 0; j < 4; j++) {
        for (size_t i = 0; i < 4; i++) {

            BOOST_CHECK_EQUAL( prop.iget( i, j, 0 ), 4 );
            BOOST_CHECK_EQUAL( prop.iget( i, j, 1 ), 2 );
        }
    }
}

BOOST_AUTO_TEST_CASE(ADD) {
    typedef Opm::GridProperty<int>::SupportedKeywordInfo SupportedKeywordInfo;
    SupportedKeywordInfo keywordInfo1( "P1", 1, "1", true );
    SupportedKeywordInfo keywordInfo2( "P2", 9, "1", true );
    Opm::GridProperty<int> prop1( 4, 4, 2, keywordInfo1 );
    Opm::GridProperty<int> prop2( 4, 4, 2, keywordInfo2 );

    Opm::Box global( 4, 4, 2 );
    Opm::Box layer0( global, 0, 3, 0, 3, 0, 0 );

    prop2.copyFrom( prop1, layer0 );
    prop2.add( 2, global );
    prop2.add( 2, layer0 );

    for (size_t j = 0; j < 4; j++) {
        for (size_t i = 0; i < 4; i++) {

            BOOST_CHECK_EQUAL( prop2.iget( i, j, 0 ), 5 );
            BOOST_CHECK_EQUAL( prop2.iget( i, j, 1 ), 11 );
        }
    }
}

BOOST_AUTO_TEST_CASE(GridPropertyInitialization) {
    const char* deckString =
        "RUNSPEC\n"
        "\n"
        "OIL\n"
        "GAS\n"
        "WATER\n"
        "TABDIMS\n"
        "3 /\n"
        "\n"
        "METRIC\n"
        "\n"
        "DIMENS\n"
        "3 3 3 /\n"
        "\n"
        "GRID\n"
        "\n"
        "ACTNUM\n"
        " 0 8*1 0 8*1 0 8*1 /\n"
        "DXV\n"
        "1 1 1 /\n"
        "\n"
        "DYV\n"
        "1 1 1 /\n"
        "\n"
        "DZV\n"
        "1 1 1 /\n"
        "\n"
        "TOPS\n"
        "9*100 /\n"
        "\n"
        "PROPS\n"
        "\n"
        "SWOF\n"
        // table 1
        // S_w    k_r,w    k_r,o    p_c,ow
        "  0.1    0        1.0      2.0\n"
        "  0.15   0        0.9      1.0\n"
        "  0.2    0.01     0.5      0.5\n"
        "  0.93   0.91     0.0      0.0\n"
        "/\n"
        // table 2
        // S_w    k_r,w    k_r,o    p_c,ow
        "  0.00   0        1.0      2.0\n"
        "  0.05   0.01     1.0      2.0\n"
        "  0.10   0.02     0.9      1.0\n"
        "  0.15   0.03     0.5      0.5\n"
        "  0.852  1.00     0.0      0.0\n"
        "/\n"
        // table 3
        // S_w    k_r,w    k_r,o    p_c,ow
        "  0.00   0.00     0.9      2.0\n"
        "  0.05   0.02     0.8      1.0\n"
        "  0.10   0.03     0.5      0.5\n"
        "  0.801  1.00     0.0      0.0\n"
        "/\n"
        "\n"
        "SGOF\n"
        // table 1
        // S_g    k_r,g    k_r,o    p_c,og
        "  0.00   0.00     0.9      2.0\n"
        "  0.05   0.02     0.8      1.0\n"
        "  0.10   0.03     0.5      0.5\n"
        "  0.80   1.00     0.0      0.0\n"
        "/\n"
        // table 2
        // S_g    k_r,g    k_r,o    p_c,og
        "  0.05   0.00     1.0      2\n"
        "  0.10   0.02     0.9      1\n"
        "  0.15   0.03     0.5      0.5\n"
        "  0.85   1.00     0.0      0\n"
        "/\n"
        // table 3
        // S_g    k_r,g    k_r,o    p_c,og
        "  0.1    0        1.0      2\n"
        "  0.15   0        0.9      1\n"
        "  0.2    0.01     0.5      0.5\n"
        "  0.9    0.91     0.0      0\n"
        "/\n"
        "\n"
        "SWU\n"
        "* /\n"
        "\n"
        "ISGU\n"
        "* /\n"
        "\n"
        "SGCR\n"
        "* /\n"
        "\n"
        "ISGCR\n"
        "* /\n"
        "\n"
        "REGIONS\n"
        "\n"
        "SATNUM\n"
        "9*1 9*2 9*3 /\n"
        "\n"
        "IMBNUM\n"
        "9*3 9*2 9*1 /\n"
        "\n"
        "SOLUTION\n"
        "\n"
        "SCHEDULE\n";

    Opm::Parser parser;

    auto deck = parser.parseString(deckString);
    Opm::TableManager tm(deck);
    Opm::EclipseGrid eg(deck);
    Opm::Eclipse3DProperties props(deck, tm, eg);

    // make sure that Eclipse3DProperties throws if it is bugged about an _unsupported_ keyword
    BOOST_CHECK_THROW(props.hasDeckIntGridProperty("ISWU"), std::logic_error);
    BOOST_CHECK_THROW(props.hasDeckDoubleGridProperty("FLUXNUM"), std::logic_error);

    // make sure that Eclipse3DProperties does not throw if it is asked for a supported
    // grid property that is not contained in the deck
    BOOST_CHECK_NO_THROW(props.hasDeckDoubleGridProperty("ISWU"));
    BOOST_CHECK_NO_THROW(props.hasDeckIntGridProperty("FLUXNUM"));

    BOOST_CHECK(!props.hasDeckDoubleGridProperty("ISWU"));
    BOOST_CHECK(!props.hasDeckIntGridProperty("FLUXNUM"));

    BOOST_CHECK(props.hasDeckIntGridProperty("SATNUM"));
    BOOST_CHECK(props.hasDeckIntGridProperty("IMBNUM"));

    BOOST_CHECK(props.hasDeckDoubleGridProperty("SWU"));
    BOOST_CHECK(props.hasDeckDoubleGridProperty("ISGU"));
    BOOST_CHECK(props.hasDeckDoubleGridProperty("SGCR"));
    BOOST_CHECK(props.hasDeckDoubleGridProperty("ISGCR"));

    const auto& swuPropData = props.getDoubleGridProperty("SWU").getData();
    BOOST_CHECK_EQUAL(swuPropData[0 * 3*3], 0.93);
    BOOST_CHECK_EQUAL(swuPropData[1 * 3*3], 0.852);
    BOOST_CHECK_EQUAL(swuPropData[2 * 3*3], 0.801);

    const auto& sguPropData = props.getDoubleGridProperty("ISGU").getData();
    BOOST_CHECK_EQUAL(sguPropData[0 * 3*3], 0.9);
    BOOST_CHECK_EQUAL(sguPropData[1 * 3*3], 0.85);
    BOOST_CHECK_EQUAL(sguPropData[2 * 3*3], 0.80);


    const auto& satnum = props.getIntGridProperty("SATNUM");
    {
        const auto& activeMap = eg.getActiveMap();
        const auto cells1 = satnum.cellsEqual(1 , activeMap);
        const auto cells2 = satnum.cellsEqual(2 , activeMap);
        const auto cells3 = satnum.cellsEqual(3 , activeMap);

        BOOST_CHECK_EQUAL( cells1.size() , 8 );
        BOOST_CHECK_EQUAL( cells2.size() , 8 );
        BOOST_CHECK_EQUAL( cells3.size() , 8 );

        for (size_t i = 0; i < 8; i++) {
            BOOST_CHECK_EQUAL( cells1[i] , i );
            BOOST_CHECK_EQUAL( cells2[i] , i + 8);
            BOOST_CHECK_EQUAL( cells3[i] , i + 16);
        }
    }
    {
        const auto cells1 = satnum.indexEqual(1 );
        const auto cells2 = satnum.indexEqual(2 );
        const auto cells3 = satnum.indexEqual(3 );

        BOOST_CHECK_EQUAL( cells1.size() , 9 );
        BOOST_CHECK_EQUAL( cells2.size() , 9 );
        BOOST_CHECK_EQUAL( cells3.size() , 9 );

        for (size_t i = 0; i < 9; i++) {
            BOOST_CHECK_EQUAL( cells1[i] , i );
            BOOST_CHECK_EQUAL( cells2[i] , i + 9);
            BOOST_CHECK_EQUAL( cells3[i] , i + 18);
        }
    }

    {
        const auto cells3_a = satnum.cellsEqual(3 , eg);
        const auto cells3_g = satnum.cellsEqual(3 , eg , false);

        for (size_t i = 0; i < 8; i++) {
            BOOST_CHECK_EQUAL( cells3_a[i] , i + 16);
            BOOST_CHECK_EQUAL( cells3_g[i] , i + 18);
        }
        BOOST_CHECK_EQUAL( cells3_g[8] , 26);
    }

    const auto compressedSatnum = satnum.compressedCopy( eg );
    BOOST_CHECK_EQUAL( compressedSatnum.size() , eg.getNumActive());
    for (size_t i=0; i < eg.getNumActive(); i++) {
        size_t g = eg.getGlobalIndex( i );
        BOOST_CHECK_EQUAL( compressedSatnum[i] , satnum.getData()[g]);
    }

    {
        const auto& double_props = props.getDoubleProperties( );
        BOOST_CHECK( !double_props.hasKeyword( "NTG" ));
        double_props.assertKeyword("NTG");
        BOOST_CHECK( double_props.hasKeyword( "NTG" ));
    }
}


inline void TestPostProcessorMul(std::vector< double >& values,
        const Opm::TableManager*,
        const Opm::EclipseGrid*,
        Opm::GridProperties<int>*,
        Opm::GridProperties<double>*)
{
    for( size_t g = 0; g < values.size(); g++ )
        values[g] *= 2.0;
}


BOOST_AUTO_TEST_CASE(multiply) {
    typedef Opm::GridProperty<int>::SupportedKeywordInfo SupportedKeywordInfo;
    SupportedKeywordInfo keywordInfo("P" , 10 , "1");
    Opm::GridProperty<int> p1( 5 , 5 , 4 , keywordInfo);
    Opm::GridProperty<int> p2( 5 , 5 , 5 , keywordInfo);
    Opm::GridProperty<int> p3( 5 , 5 , 4 , keywordInfo);

    BOOST_CHECK_THROW( p1.multiplyWith(p2) , std::invalid_argument );
    p1.multiplyWith(p3);

    for (size_t g = 0; g < p1.getCartesianSize(); g++)
        BOOST_CHECK_EQUAL( 100 , p1.iget(g));

}



BOOST_AUTO_TEST_CASE(mask_test) {
    typedef Opm::GridProperty<int>::SupportedKeywordInfo SupportedKeywordInfo;
    SupportedKeywordInfo keywordInfo1("P" , 10 , "1");
    SupportedKeywordInfo keywordInfo2("P" , 20 , "1");
    Opm::GridProperty<int> p1( 5 , 5 , 4 , keywordInfo1);
    Opm::GridProperty<int> p2( 5 , 5 , 4 , keywordInfo2);

    std::vector<bool> mask;

    p1.initMask(10 , mask);
    p2.maskedSet( 10 , mask);

    for (size_t g = 0; g < p1.getCartesianSize(); g++)
        BOOST_CHECK_EQUAL( p1.iget(g) , p2.iget(g));
}

BOOST_AUTO_TEST_CASE(CheckLimits) {
    typedef Opm::GridProperty<int>::SupportedKeywordInfo SupportedKeywordInfo;
    SupportedKeywordInfo keywordInfo1("P" , 1 , "1");
    Opm::GridProperty<int> p1( 5 , 5 , 4 , keywordInfo1);

    p1.checkLimits(0,2);
    BOOST_CHECK_THROW( p1.checkLimits(-2,0) , std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(PropertiesEmpty) {
    typedef Opm::GridProperties<int>::SupportedKeywordInfo SupportedKeywordInfo;
    std::vector<SupportedKeywordInfo> supportedKeywords = {
        SupportedKeywordInfo("SATNUM" , 0, "1"),
        SupportedKeywordInfo("FIPNUM" , 2, "1")
    };

    const Opm::EclipseGrid grid(10, 7, 9);
    Opm::GridProperties<int> gridProperties(grid, std::move(supportedKeywords));

    BOOST_CHECK( gridProperties.supportsKeyword("SATNUM") );
    BOOST_CHECK( gridProperties.supportsKeyword("FIPNUM") );
    BOOST_CHECK( !gridProperties.supportsKeyword("FLUXNUM") );
    BOOST_CHECK( !gridProperties.hasKeyword("SATNUM"));
    BOOST_CHECK( !gridProperties.hasKeyword("FLUXNUM"));

    BOOST_CHECK_THROW( gridProperties.getDeckKeyword("SATNUM") , std::invalid_argument);
    BOOST_CHECK_THROW( gridProperties.getDeckKeyword("NONONO") , std::invalid_argument);
}



BOOST_AUTO_TEST_CASE(addKeyword) {
    typedef Opm::GridProperties<int>::SupportedKeywordInfo SupportedKeywordInfo;
    std::vector<SupportedKeywordInfo> supportedKeywords = {
        SupportedKeywordInfo("SATNUM" , 0, "1")
    };
    Opm::EclipseGrid grid(10,7,9);
    Opm::GridProperties<int> gridProperties(grid, std::move( supportedKeywords ));

    BOOST_CHECK_THROW( gridProperties.addKeyword("NOT-SUPPORTED"), std::invalid_argument);

    BOOST_CHECK(  gridProperties.addKeyword("SATNUM"));
    BOOST_CHECK( !gridProperties.addKeyword("SATNUM"));
    BOOST_CHECK(  gridProperties.hasKeyword("SATNUM"));
}


BOOST_AUTO_TEST_CASE(hasKeyword_assertKeyword) {
    typedef Opm::GridProperties<int>::SupportedKeywordInfo SupportedKeywordInfo;
    std::vector<SupportedKeywordInfo> supportedKeywords = {
        SupportedKeywordInfo("SATNUM" , 0, "1", true),
        SupportedKeywordInfo("FIPNUM" , 0, "1", true)
    };
    const Opm::EclipseGrid grid(10, 7, 9);
    const Opm::GridProperties<int> gridProperties( grid, std::move( supportedKeywords ) );

    // calling getKeyword() should not change the semantics of hasKeyword()!
    BOOST_CHECK(!gridProperties.hasKeyword("SATNUM"));
    BOOST_CHECK(!gridProperties.hasKeyword("FIPNUM"));

    gridProperties.assertKeyword("FIPNUM");
    gridProperties.getKeyword("SATNUM");
    BOOST_CHECK(gridProperties.hasKeyword("SATNUM"));
    BOOST_CHECK(gridProperties.hasKeyword("FIPNUM"));

    BOOST_CHECK_THROW( gridProperties.getKeyword( "NOT-SUPPORTED" ), std::invalid_argument );
}
