// --------------------------------------------------------------------------
//                   OpenMS -- Open-Source Mass Spectrometry
// --------------------------------------------------------------------------
// Copyright The OpenMS Team -- Eberhard Karls University Tuebingen,
// ETH Zurich, and Freie Universitaet Berlin 2002-2018.
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
// $Authors:  Dominik Schmitz, Chris Bielow$
// --------------------------------------------------------------------------

#pragma once

#include <OpenMS/KERNEL/FeatureMap.h>
#include <OpenMS/FORMAT/FASTAFile.h>
#include <OpenMS/METADATA/PeptideIdentification.h>
#include <algorithm>
#include <bits/stdc++.h>


namespace OpenMS
{
  class OPENMS_DLLAPI Contaminants
  {
  public:
    void compute(FeatureMap& features, const std::vector<FASTAFile::FASTAEntry>& contaminants);
    const std::vector<std::pair<double, double>>& getResults();
    struct resultsData
    {
      double assigned_contaminants_ratio; //(# contaminants in assigned/ #peptides in assigned)
      double unassigned_contaminants_ratio; //(# contaminants in unassigned/ #peptides in unassigned)
      double all_contaminants_ratio; //(# all contaminants/ #peptides in all)
      double assigned_contaminants_intensity; //(intensity of contaminants in assigned/ intensity of peptides in assigned)
    };
  private:
    std::vector<std::pair<double, double>> results_;
    std::unordered_set<String> digested_db_;
    void compare(const String& key, Feature& f, Int64& total, Int64& cont, double& sum_total, double& sum_cont);
  };
}
