/*
 * Copyright (c) 2019, Intel Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

//!
//! \file:   MpdGenerator.h
//! \brief:  Mpd generator class definition
//! \detail: Define the operation and needed data for mpd generator, including
//!          writing and updating mpd file according to segmentation informaton,
//!          called by Segmentation.
//!
//! Created on April 30, 2019, 6:04 AM
//!

#ifndef _MPDGENERATOR_H_
#define _MPDGENERATOR_H_

//#include "definitions.h"
#include "MediaStream.h"
#include "ExtractorTrackManager.h"
#include "DashSegmenter.h"
#include "../utils/OmafStructure.h"
#include "../utils/tinyxml2.h"

VCD_NS_BEGIN

using namespace tinyxml2;

//!
//! \class MpdGenerator
//! \brief Define the operation and needed data for mpd generator
//!

class MpdGenerator
{
public:
    //!
    //! \brief  Constructor
    //!
    MpdGenerator();

    //!
    //! \brief  Copy Constructor
    //!
    //! \param  [in] streamsSegCtxs
    //!         pointer to map of media stream and its
    //!         track segmentation context
    //! \param  [in] extractorSegCtxs
    //!         pointer to map of extractor track and its
    //!         track segmentation context
    //! \param  [in] segInfo
    //!         pointer to the segmentation information
    //! \param  [in] projType
    //!         projection type
    //! \param  [in] frameRate
    //!         video stream frame rate
    //!
    MpdGenerator(
        std::map<MediaStream*, TrackSegmentCtx*> *streamsSegCtxs,
        std::map<ExtractorTrack*, TrackSegmentCtx*> *extractorSegCtxs,
        SegmentationInfo *segInfo,
        VCD::OMAF::ProjectionFormat projType,
        Rational frameRate);


    //!
    //! \brief  Destructor
    //!
    ~MpdGenerator();

    //!
    //! \brief  Initialize the mpd generator, including
    //!         collecting region wise packing information,
    //!         and content coverage information for all
    //!         extractor tracks.
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t Initialize();

    //!
    //! \brief  Write the mpd file according to segmentation information
    //!
    //! \param  [in] totalFramesNum
    //!         total number of frames written into segments
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t WriteMpd(uint64_t totalFramesNum);

    //!
    //! \brief  Update the mpd file according to segmentation information
    //!
    //! \param  [in] segNumber
    //!         total number of segments have been written
    //! \param  [in] framesNumber
    //!         total number of frames have been written into segments
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t UpdateMpd(uint64_t segNumber, uint64_t framesNumber);

private:

    //!
    //! \brief  Write AdaptationSet for tile track in mpd file
    //!
    //! \param  [in] periodEle
    //!         pointer to period element has been create for
    //!         mpd file using tinyxml2
    //! \param  [in] pTrackSegCtx
    //!         pointer to track segmentation context for tile track
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t WriteTileTrackAS(XMLElement *periodEle, TrackSegmentCtx *pTrackSegCtx);

    //!
    //! \brief  Write AdaptationSet for extractor track in mpd file
    //!
    //! \param  [in] periodEle
    //!         pointer to period element has been create for
    //!         mpd file using tinyxml2
    //! \param  [in] pTrackSegCtx
    //!         pointer to track segmentation context for extractor track
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t WriteExtractorTrackAS(XMLElement *periodEle, TrackSegmentCtx *pTrackSegCtx);

private:
    std::map<MediaStream*, TrackSegmentCtx*>    *m_streamSegCtx;    //!< map of media stream and its track segmentation context
    std::map<ExtractorTrack*, TrackSegmentCtx*> *m_extractorSegCtx; //!< map of extractor track and its track segmentation context
    //std::map<uint8_t, MediaStream*>       *m_streamMap;          //!< media streams map set up in OmafPackage
    //ExtractorTrackManager                 *m_extractorTrackMan;  //!< pointer to the extractor track manager created in OmafPackage
    SegmentationInfo                            *m_segInfo;            //!< pointer to the segmentation information
    uint32_t                                    m_miniUpdatePeriod;    //!< minimum update period of mpd file, in the unit of second
    VCD::OMAF::ProjectionFormat                 m_projType;            //!< projection type of the video frame
    char                                        m_availableStartTime[1024]; //!< available start time for mpd file for live streaming
    char                                        *m_publishTime;        //!< publish time for mpd file
    char                                        *m_presentationDur;    //!< presentation duration of dash segments

    //std::map<uint8_t, RegionWisePacking*> m_rwpk;                //!< region wise packing information map for all extractor tracks
    //std::map<uint8_t, ContentCoverage*>   m_covi;                //!< content coverage information map for all extractor tracks
    char                                        m_mpdFileName[1024];   //!< file name of MPD file
    Rational                                    m_frameRate;           //!< video stream frame rate
    uint16_t                                    m_timeScale;           //!< timescale of video stream
    XMLDocument                                 *m_xmlDoc;             //!< XML doc element for writting mpd file created using tinyxml2
};

VCD_NS_END;
#endif /* _MPDGENERATOR_H_ */
