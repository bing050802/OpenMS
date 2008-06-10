// -*- Mode: C++; tab-width: 2; -*-
// vi: set ts=2:
//
// --------------------------------------------------------------------------
//                   OpenMS Mass Spectrometry Framework
// --------------------------------------------------------------------------
//  Copyright (C) 2003-2008 -- Oliver Kohlbacher, Knut Reinert
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// --------------------------------------------------------------------------
// $Maintainer: Eva Lange $
// --------------------------------------------------------------------------
//

#ifndef OPENMS_FILTERING_BASELINE_TOPHATFILTER_H
#define OPENMS_FILTERING_BASELINE_TOPHATFILTER_H

#include <OpenMS/FILTERING/BASELINE/MorphFilter.h>
#include <OpenMS/KERNEL/MSExperiment.h>
#include <OpenMS/MATH/MISC/MathFunctions.h>

#include <algorithm>

namespace OpenMS
{

  /**
  	@brief This class represents a Top Hat baseline filter.
   
    This filter can be used by supposing that the required lineaments are brighter than the environment.
    The main advantage of this filter is to be able to detect an over brightness even if the environment is not uniform.
    Moreover it is possible to regulate the size or the width of the over brightnesses very easily.
    The principle is based on the subtraction of an signal \f$ s \f$  from its opening  \f$ \gamma \f$.
    The opening consists of an erosion followed by a dilation,
    the size (the frameSize) of the structuring element (here a flat line) being conditioned by the width of the lineament
    to be detected.
    
    @note This filter works only for uniform raw data!
    
    @ref TopHatFilter_Parameters are explained on a separate page.
    
		@ingroup SignalProcessing
  */
  class TopHatFilter 
  	: public MorphFilter
  {
    public:
      typedef MorphFilter BaseClass;
      using BaseClass::struc_size_;

      /// Constructor
      inline TopHatFilter() 
      	: MorphFilter()
      {
      }

      /// Destructor
      virtual ~TopHatFilter()
      {
      }

      /** 
      	@brief Applies the baseline removal algorithm to an given iterator range.

        Removes the baseline in the given iterator intervall [first,last) and writes the
        resulting data to the baseline_filtered_container.
        
        @note This method assumes that the InputPeakIterator (e.g. of type MSSpectrum<Peak1D >::const_iterator)
              points to a data point of type Peak1D or any other class derived from Peak1D.
        
        @note The resulting peaks in the baseline_filtered_container (e.g. of type MSSpectrum<Peak1D >)
              can be of type Peak1D or any other class derived from DPeak.
      */
      template <typename InputPeakIterator, typename OutputPeakContainer  >
      void filter(InputPeakIterator first, InputPeakIterator last, OutputPeakContainer& baseline_filtered_container)
      {
        typedef typename InputPeakIterator::value_type PeakType;

        // compute the number of data points of the structuring element given the spacing of the raw data
        // and the size (in Th) of the structuring element
        DoubleReal spacing= ((last-1)->getMZ() - first->getMZ()) / (distance(first,last)-1);
        int struc_elem_number_of_points = (int) ceil(struc_size_ / spacing );

        // the number has to be odd
        if (!Math::isOdd(struc_elem_number_of_points))
        {
          struc_elem_number_of_points += 1;
        }
//         std::cout << "struc_size_ " << struc_size_ 
//                   <<  " spacing " << spacing 
//                   << " (struc_size_ / spacing ) " << (struc_size_ / spacing )
//                   << " round(struc_size_ / spacing ) " << ceil(struc_size_ / spacing ) 
//                   << " struc_elem_number_of_points " << struc_elem_number_of_points << std::endl;

        std::vector<PeakType> erosion_result;
        // compute the erosion of raw data
        this->template erosion//<InputPeakIterator>
        (first, last, erosion_result, struc_elem_number_of_points);
        // compute the dilation of erosion_result
        this->template dilatation//<InputPeakIterator>
        (erosion_result.begin(),erosion_result.end(), baseline_filtered_container, struc_elem_number_of_points);
        // subtract the result from the original data
        this->template minusIntensities_//<InputPeakIterator>
        (first,last,baseline_filtered_container);
      }


      /** 
      	@brief Applies the baseline removal algorithm to to a raw data point container.

        Removes the baseline in the the input container (e.g. of type MSSpectrum<Peak1D >) and writes the 
        resulting data to the baseline_filtered_container.
        
        @note This method assumes that the InputPeakIterator (e.g. of type MSSpectrum<Peak1D >::const_iterator)
              points to a data point of type Peak1D or any other class derived from Peak1D.
        
        @note The resulting peaks in the baseline_filtered_container (e.g. of type MSSpectrum<Peak1D >)
              can be of type Peak1D or any other class derived from DPeak. 
      */
      template <typename InputPeakContainer, typename OutputPeakContainer >
      void filter(const InputPeakContainer& input_peak_container, OutputPeakContainer& baseline_filtered_container)
      {
        // copy the experimental settings
        static_cast<SpectrumSettings&>(baseline_filtered_container) = input_peak_container;
        
        filter(input_peak_container.begin(), input_peak_container.end(), baseline_filtered_container);
      }


      /** 
      	@brief Removes the baseline in a range of MSSpectra.
          
        Filters the data successive in every scan in the intervall [first,last).
        The filtered data are stored in a MSExperiment.
                
        @note The InputSpectrumIterator should point to a MSSpectrum. Elements of the input spectra should be of type Peak1D 
                or any other derived class of DPeak.

        @note You have to copy the ExperimentalSettings of the raw data by your own.  
      */
      template <typename InputSpectrumIterator, typename OutputPeakType, typename OutputAllocType >
      void filterExperiment(InputSpectrumIterator first, InputSpectrumIterator last, MSExperiment<OutputPeakType, OutputAllocType>& ms_exp_filtered)
      {
        UInt n = distance(first,last);
        ms_exp_filtered.reserve(n);
        startProgress(0,n,"filtering baseline of data");
        // pick peaks on each scan
        for (UInt i = 0; i < n; ++i)
        {
          InputSpectrumIterator input_it(first+i);
          // if the scan contains enough raw data points filter the baseline
          if ( struc_size_ < fabs((input_it->end()-1)->getMZ()- input_it->begin()->getMZ()))
          {
            MSSpectrum< OutputPeakType > spectrum;

            // pick the peaks in scan i
            filter(*input_it,spectrum);
            setProgress(i);

            // if any peaks are found copy the spectrum settings
            if (spectrum.size() > 0)
            {
              // copy the spectrum settings
              static_cast<SpectrumSettings&>(spectrum) = *input_it;
              spectrum.setType(SpectrumSettings::RAWDATA);

              // copy the spectrum information
              spectrum.getPrecursorPeak() = input_it->getPrecursorPeak();
              spectrum.setRT(input_it->getRT());
              spectrum.setMSLevel(input_it->getMSLevel());
              spectrum.getName() = input_it->getName();

              ms_exp_filtered.push_back(spectrum);
            }
          }
        }
        endProgress();
      }

      /** 
      	@brief Removes the baseline in a MSExperiment.
        
	      Filters the data every scan in the MSExperiment.
	      The filtered data are stored in a MSExperiment.
	              
	      @note The InputSpectrumIterator should point to a MSSpectrum. Elements of the input spectra should be of type Peak1D 
	            or any other derived class of DPeak.
      */
      template <typename InputPeakType, typename InputAllocType, typename OutputPeakType,  typename OutputAllocType >
      void filterExperiment(const MSExperiment< InputPeakType, InputAllocType>& ms_exp_raw, MSExperiment<OutputPeakType, OutputAllocType>& ms_exp_filtered)
      {
        // copy the experimental settings
        static_cast<ExperimentalSettings&>(ms_exp_filtered) = ms_exp_raw;

        filterExperiment(ms_exp_raw.begin(), ms_exp_raw.end(), ms_exp_filtered);
      }
  };

}// namespace OpenMS
#endif
