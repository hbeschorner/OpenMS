// --------------------------------------------------------------------------
//                   OpenMS -- Open-Source Mass Spectrometry               
// --------------------------------------------------------------------------
// Copyright The OpenMS Team -- Eberhard Karls University Tuebingen,
// ETH Zurich, and Freie Universitaet Berlin 2002-2021.
// 
// This software is released under a three-clause BSD license:
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of any author or any participating institution 
//    may be used to endorse or promote products derived from this software 
//    without specific prior written permission.
// For a full list of authors, refer to the file AUTHORS. 
// --------------------------------------------------------------------------
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL ANY OF THE AUTHORS OR THE CONTRIBUTING 
// INSTITUTIONS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// --------------------------------------------------------------------------
// $Maintainer: Timo Sachsenberg $
// $Authors: Eva Lange $
// --------------------------------------------------------------------------

#include <OpenMS/CONCEPT/ClassTest.h>
#include <OpenMS/test_config.h>

///////////////////////////

#include <OpenMS/FILTERING/SMOOTHING/SavitzkyGolayFilter.h>
#include <OpenMS/KERNEL/Peak2D.h>

///////////////////////////

START_TEST(SavitzkyGolayFilter<D>, "$Id$")

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

using namespace OpenMS;

SavitzkyGolayFilter* dsg_ptr = nullptr;
SavitzkyGolayFilter* dsg_nullPointer = nullptr;
START_SECTION((SavitzkyGolayFilter()))
  dsg_ptr = new SavitzkyGolayFilter;
  TEST_NOT_EQUAL(dsg_ptr, dsg_nullPointer)
END_SECTION

START_SECTION((virtual ~SavitzkyGolayFilter()))
  delete dsg_ptr;
END_SECTION

Param param;
param.setValue("polynomial_order",2);
param.setValue("frame_length",3);

START_SECTION((template < typename PeakType > void filter(MSSpectrum &spectrum)))
  MSSpectrum spectrum;
  spectrum.resize(5);
  MSSpectrum::Iterator it = spectrum.begin();
  for (int i=0; i<5; ++i, ++it)
  {
  	it->setIntensity(0.0f);
    if (i==2)
    {
      it->setIntensity(1.0f);
    }
  }

  SavitzkyGolayFilter sgolay;
	sgolay.setParameters(param);
  sgolay.filter(spectrum);
  
  it=spectrum.begin();
  TEST_REAL_SIMILAR(it->getIntensity(),0.0)
  ++it;
  TEST_REAL_SIMILAR(it->getIntensity(),0.0)
  ++it;
  TEST_REAL_SIMILAR(it->getIntensity(),1.0)
  ++it;
  TEST_REAL_SIMILAR(it->getIntensity(),0.0)
  ++it;
  TEST_REAL_SIMILAR(it->getIntensity(),0.0)
END_SECTION 


START_SECTION((template <typename PeakType> void filterExperiment(MSExperiment<PeakType>& map)))
	TOLERANCE_ABSOLUTE(0.01)

	param.setValue("frame_length",4);

	PeakMap exp;
  exp.resize(4);

  Peak1D p;
  for (int i=0; i<9; ++i)
  {
  	p.setIntensity(0.0f);
    if (i==3)
    {
  		p.setIntensity(1.0f);
    }
    if (i==4)
    {
  		p.setIntensity(0.8f);
    }
    if (i==5)
    {
  		p.setIntensity(1.2f);
    }
    exp[0].push_back(p);
    exp[1].push_back(p);
  }
  exp[2].push_back(p);

  SavitzkyGolayFilter sgolay;
	sgolay.setParameters(param);
  sgolay.filterExperiment(exp);

	TEST_EQUAL(exp.size(),4)
	TEST_EQUAL(exp[0].size(),9)
	TEST_EQUAL(exp[1].size(),9)
	TEST_EQUAL(exp[2].size(),1)
	TEST_EQUAL(exp[3].size(),0)

	TEST_REAL_SIMILAR(exp[0][0].getIntensity(),0.0)
	TEST_REAL_SIMILAR(exp[0][1].getIntensity(),0.0571429)
	TEST_REAL_SIMILAR(exp[0][2].getIntensity(),0.274286)
	TEST_REAL_SIMILAR(exp[0][3].getIntensity(),0.657143)
	TEST_REAL_SIMILAR(exp[0][4].getIntensity(),1.14286)
	TEST_REAL_SIMILAR(exp[0][5].getIntensity(),0.771429)
	TEST_REAL_SIMILAR(exp[0][6].getIntensity(),0.342857)
	TEST_REAL_SIMILAR(exp[0][7].getIntensity(),0.0914286)
	TEST_REAL_SIMILAR(exp[0][8].getIntensity(),0.0)

	TEST_REAL_SIMILAR(exp[1][0].getIntensity(),0.0)
	TEST_REAL_SIMILAR(exp[1][1].getIntensity(),0.0571429)
	TEST_REAL_SIMILAR(exp[1][2].getIntensity(),0.274286)
	TEST_REAL_SIMILAR(exp[1][3].getIntensity(),0.657143)
	TEST_REAL_SIMILAR(exp[1][4].getIntensity(),1.14286)
	TEST_REAL_SIMILAR(exp[1][5].getIntensity(),0.771429)
	TEST_REAL_SIMILAR(exp[1][6].getIntensity(),0.342857)
	TEST_REAL_SIMILAR(exp[1][7].getIntensity(),0.0914286)
	TEST_REAL_SIMILAR(exp[1][8].getIntensity(),0.0)

	TEST_REAL_SIMILAR(exp[2][0].getIntensity(),0.0)

END_SECTION

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
END_TEST
