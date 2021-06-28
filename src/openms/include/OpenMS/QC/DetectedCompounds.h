// --------------------------------------------------------------------------
//                   OpenMS -- Open-Source Mass Spectrometry
// --------------------------------------------------------------------------
// Copyright The OpenMS Team -- Eberhard Karls University Tuebingen,
// ETH Zurich, and Freie Universitaet Berlin 2002-2020.
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
// $Maintainer: Chris Bielow $
// $Authors: Tom Waschischeck $
// --------------------------------------------------------------------------

#pragma once

#include <OpenMS/QC/QCBase.h>

/**
 * @brief Detected Compounds as a Metabolomics QC metric
 *
 * Simple class to return the number of detected compounds
 * from a given library of target compounds in a specific run.
 *
 */

namespace OpenMS
{
  class OPENMS_DLLAPI DetectedCompounds : public QCBase
  {
  public:
    /// Constructor
    DetectedCompounds() = default;

    /// Destructor
    virtual ~DetectedCompounds() = default;

    // stores DetectedCompounds values calculated by compute function
    struct OPENMS_DLLAPI Result
    {
      int detected_compounds = 0;
      float rt_shift_mean = 0;

      bool operator==(const Result& rhs) const;
    };

     /**
    @brief computes the number of detected compounds in a featureXML file

    @param featureXML featureXML file created by FeatureFinderMetaboIdent based on a given library of target compounds
    @return number of detected compounds from a given library of target compounds in a specific run

    **/
    Result compute(const String& pathToFeatureXMLFile);

    const String& getName() const override;

    QCBase::Status requires() const override;

  private:
    const String name_ = "Detected Compounds";
  };
}
