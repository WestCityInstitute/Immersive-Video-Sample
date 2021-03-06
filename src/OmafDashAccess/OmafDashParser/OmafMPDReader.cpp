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
//! \file:   OmafMPDReader.cpp
//! \brief:  parse XML tree to get MPD tree with OMAF DASH standard
//!

#include "OmafMPDReader.h"

VCD_OMAF_BEGIN


OmafMPDReader::OmafMPDReader()
{
    m_rootXMLElement = nullptr;
    m_mpd = nullptr;
}

OmafMPDReader::OmafMPDReader(OmafXMLElement *root):OmafMPDReader()
{
    m_rootXMLElement = root;
}

OmafMPDReader::~OmafMPDReader()
{
    SAFE_DELETE(m_rootXMLElement);
    SAFE_DELETE(m_mpd);
}

ODStatus OmafMPDReader::Init()
{
    return OD_STATUS_SUCCESS;
}

void OmafMPDReader::Close()
{}

ODStatus OmafMPDReader::BuildMPD()
{
    if(!m_rootXMLElement)
        return OD_STATUS_INVALID;

    m_mpd = new MPDElement();
    CheckNullPtr_PrintLog_ReturnStatus(m_mpd, "Failed to create MPD element.", ERROR, OD_STATUS_OPERATION_FAILED);

    // read MPD attributes in XML
    m_mpd->SetXmlnsOmaf(m_rootXMLElement->GetAttributeVal(OMAF_XMLNS));
    m_mpd->SetXmlnsXsi(m_rootXMLElement->GetAttributeVal(XSI_XMLNS));
    m_mpd->SetXmlns(m_rootXMLElement->GetAttributeVal(XMLNS));
    m_mpd->SetXmlnsXlink(m_rootXMLElement->GetAttributeVal(XLINK_XMLNS));
    m_mpd->SetXsiSchemaLocation(m_rootXMLElement->GetAttributeVal(XSI_SCHEMALOCATION));
    m_mpd->SetMinBufferTime(m_rootXMLElement->GetAttributeVal(MINBUFFERTIME));
    m_mpd->SetMaxSegmentDuration(m_rootXMLElement->GetAttributeVal(MAXSEGMENTDURATION));
    m_mpd->AddProfile(m_rootXMLElement->GetAttributeVal(PROFILES));
    m_mpd->SetType(m_rootXMLElement->GetAttributeVal(MPDTYPE));
    m_mpd->SetAvailabilityStartTime(m_rootXMLElement->GetAttributeVal(AVAILABILITYSTARTTIME));
    m_mpd->SetTimeShiftBufferDepth(m_rootXMLElement->GetAttributeVal(TIMESHIFTBUFFERDEPTH));
    m_mpd->SetMinimumUpdatePeriod(m_rootXMLElement->GetAttributeVal(MINIMUMUPDATEPERIOD));
    m_mpd->SetPublishTime(m_rootXMLElement->GetAttributeVal(PUBLISHTIME));
    m_mpd->SetMediaPresentationDuration(m_rootXMLElement->GetAttributeVal(MEDIAPRESENTATIONDURATION));

    map<string, string> attributes = m_rootXMLElement->GetAttributes();
    m_mpd->AddOriginalAttributes(attributes);

    CheckNullPtr_PrintLog_ReturnStatus(m_rootXMLElement, "Failed to create MPD node.", ERROR, OD_STATUS_OPERATION_FAILED);
    vector<OmafXMLElement*> childElement = m_rootXMLElement->GetChildElements();
    for(auto child : childElement)
    {
        if(!child)
        {
            LOG(WARNING)<<"Faild to load sub element in MPD Element."<<endl;
            continue;
        }
        if(child->GetName() == "EssentialProperty")
        {
            EssentialPropertyElement* essentialProperty = nullptr;
            essentialProperty = BuildEssentialProperty(child);
            if(essentialProperty)
                m_mpd->AddEssentialProperty(essentialProperty);
            else
                LOG(WARNING)<<"Faild to set EssentialProperty."<<endl;
        }
        else if(child->GetName() == "BaseURL")
        {
            BaseUrlElement* baseURL = nullptr;
            baseURL = BuildBaseURL(child);
            if(baseURL)
                m_mpd->AddBaseUrl(baseURL);
            else
                LOG(WARNING)<<"Faild to add baseURL."<<endl;
        }
        else if(child->GetName() == "Period")
        {
            PeriodElement* period = nullptr;
            period = BuildPeriod(child);
            if(period)
                m_mpd->AddPeriod(period);
            else
                LOG(WARNING)<<"Faild to add period."<<endl;
        }
        else
        {
            LOG(INFO)<<"Can't parse element in BuildMPD."<<endl;
        }
        m_mpd->AddChildElement(child);
    }

    m_mpd->AddBaseUrl(BuildBaseURL(m_rootXMLElement));

    return OD_STATUS_SUCCESS;
}

BaseUrlElement* OmafMPDReader::BuildBaseURL(OmafXMLElement* xmlBaseURL)
{
    CheckNullPtr_PrintLog_ReturnNullPtr(xmlBaseURL, "Failed to read baseURL element.", ERROR);
    BaseUrlElement* baseURL = new BaseUrlElement();
    CheckNullPtr_PrintLog_ReturnNullPtr(baseURL, "Failed to create baseURL node.", ERROR);

    auto path = xmlBaseURL->GetPath();
    baseURL->SetPath(path);

    map<string, string> attributes = xmlBaseURL->GetAttributes();
    baseURL->AddOriginalAttributes(attributes);

    return baseURL;
}

PeriodElement* OmafMPDReader::BuildPeriod(OmafXMLElement* xmlPeriod)
{
    CheckNullPtr_PrintLog_ReturnNullPtr(xmlPeriod, "Failed to read period element.", ERROR);
    PeriodElement* period = new PeriodElement();
    CheckNullPtr_PrintLog_ReturnNullPtr(period, "Failed to create period node.", ERROR);
    period->SetStart(xmlPeriod->GetAttributeVal(START));
    period->SetId(xmlPeriod->GetAttributeVal(INDEX));

    map<string, string> attributes = xmlPeriod->GetAttributes();
    period->AddOriginalAttributes(attributes);

    vector<OmafXMLElement*> childElement = xmlPeriod->GetChildElements();
    for(auto child : childElement)
    {
        if(!child)
        {
            LOG(WARNING)<<"Faild to load sub element in Period Element."<<endl;
            continue;
        }

        if(child->GetName() == "AdaptationSet")
        {
            AdaptationSetElement* adaptationSet = nullptr;
            adaptationSet = BuildAdaptationSet(child);
            if(adaptationSet)
                period->AddAdaptationSet(adaptationSet);
            else
                LOG(WARNING)<<"Fail to add adaptionSet."<<endl;
        }
        else
        {
            LOG(INFO)<<"Can't parse element in BuildPeriod."<<endl;
        }
        period->AddChildElement(child);
    }

    return period;
}

AdaptationSetElement* OmafMPDReader::BuildAdaptationSet(OmafXMLElement* xml)
{
    CheckNullPtr_PrintLog_ReturnNullPtr(xml, "Failed to read adaptionSet element.", ERROR);
    AdaptationSetElement* adaptionSet = new AdaptationSetElement();
    CheckNullPtr_PrintLog_ReturnNullPtr(adaptionSet, "Failed to create adaptionSet node.", ERROR);
    adaptionSet->SetId(xml->GetAttributeVal(INDEX));
    adaptionSet->SetMimeType(xml->GetAttributeVal(MIMETYPE));
    adaptionSet->SetCodecs(xml->GetAttributeVal(CODECS));
    adaptionSet->SetMaxWidth(xml->GetAttributeVal(MAXWIDTH));
    adaptionSet->SetMaxHeight(xml->GetAttributeVal(MAXHEIGHT));
    adaptionSet->SetMaxFrameRate(xml->GetAttributeVal(MAXFRAMERATE));
    adaptionSet->SetSegmentAlignment(xml->GetAttributeVal(SEGMENTALIGNMENT));
    adaptionSet->SetSubsegmentAlignment(xml->GetAttributeVal(SUBSEGMENTALIGNMENT));

    map<string, string> attributes = xml->GetAttributes();
    adaptionSet->AddOriginalAttributes(attributes);

    vector<OmafXMLElement*> childElement = xml->GetChildElements();
    for(auto child : childElement)
    {
        if(!child)
        {
            LOG(WARNING)<<"Faild to load sub element in  Element."<<endl;
            continue;
        }

        if(child->GetName() == "Representation")
        {
            RepresentationElement* representation = nullptr;
            representation = BuildRepresentation(child);
            if(representation)
                adaptionSet->AddRepresentation(representation);
            else
                LOG(WARNING)<<"Fail to add representation."<<endl;
        }
        else if(child->GetName() == "Viewport")
        {
            ViewportElement* viewport = nullptr;
            viewport = BuildViewport(child);
            if(viewport)
                adaptionSet->AddViewport(viewport);
            else
                LOG(WARNING)<<"Fail to add Viewport."<<endl;
        }
        else if(child->GetName() == "EssentialProperty")
        {
            EssentialPropertyElement* essentialProperty = nullptr;
            essentialProperty = BuildEssentialProperty(child);
            if(essentialProperty)
                adaptionSet->AddEssentialProperty(essentialProperty);
            else
                LOG(WARNING)<<"Fail to add essentialProperty."<<endl;
        }
        else if(child->GetName() == "SupplementalProperty")
        {
            SupplementalPropertyElement* supplementalProperty = nullptr;
            supplementalProperty = BuildSupplementalProperty(child);
            if(supplementalProperty)
                adaptionSet->AddSupplementalProperty(supplementalProperty);
            else
                LOG(WARNING)<<"Fail to add supplementalProperty."<<endl;
        }
        else
        {
            LOG(INFO)<<"Can't parse element in Build."<<endl;
        }
        adaptionSet->AddChildElement(child);
    }

    return adaptionSet;
}

ViewportElement* OmafMPDReader::BuildViewport(OmafXMLElement* xmlViewport)
{
    CheckNullPtr_PrintLog_ReturnNullPtr(xmlViewport, "Failed to read viewport element.", ERROR);
    ViewportElement* viewport = new ViewportElement();
    CheckNullPtr_PrintLog_ReturnNullPtr(viewport, "Failed to create viewport node.", ERROR);
    viewport->SetSchemeIdUri(xmlViewport->GetAttributeVal(SCHEMEIDURI));
    viewport->SetValue(xmlViewport->GetAttributeVal(VALUE));

    viewport->ParseSchemeIdUriAndValue();

    map<string, string> attributes = xmlViewport->GetAttributes();
    viewport->AddOriginalAttributes(attributes);

    vector<OmafXMLElement*> childElement = xmlViewport->GetChildElements();
    for(auto child : childElement)
    {
        if(!child)
        {
            LOG(WARNING)<<"Faild to load sub element in Viewport Element."<<endl;
            continue;
        }

        viewport->AddChildElement(child);
    }

    return viewport;
}

EssentialPropertyElement* OmafMPDReader::BuildEssentialProperty(OmafXMLElement* xmlEssentialProperty)
{
    CheckNullPtr_PrintLog_ReturnNullPtr(xmlEssentialProperty, "Failed to read essentialProperty element.", ERROR);

    EssentialPropertyElement* essentialProperty = new EssentialPropertyElement();
    CheckNullPtr_PrintLog_ReturnNullPtr(essentialProperty, "Failed to create essentialProperty node.", ERROR);

    essentialProperty->SetSchemeIdUri(xmlEssentialProperty->GetAttributeVal(SCHEMEIDURI));
    essentialProperty->SetValue(xmlEssentialProperty->GetAttributeVal(VALUE));
    essentialProperty->SetProjectionType(xmlEssentialProperty->GetAttributeVal(OMAF_PROJECTIONTYPE));
    essentialProperty->SetRwpkPackingType(xmlEssentialProperty->GetAttributeVal(OMAF_PACKINGTYPE));


    essentialProperty->ParseSchemeIdUriAndValue();

    map<string, string> attributes = xmlEssentialProperty->GetAttributes();
    essentialProperty->AddOriginalAttributes(attributes);

    vector<OmafXMLElement*> childElement = xmlEssentialProperty->GetChildElements();
    for(auto child : childElement)
    {
        if(!child)
        {
            LOG(WARNING)<<"Faild to load sub element in EssentialProperty Element."<<endl;
            continue;
        }

        essentialProperty->AddChildElement(child);
    }

    return essentialProperty;
}

RepresentationElement* OmafMPDReader::BuildRepresentation(OmafXMLElement* xmlRepresentation)
{
    CheckNullPtr_PrintLog_ReturnNullPtr(xmlRepresentation, "Failed to read representation element.", ERROR);
    RepresentationElement* representation = new RepresentationElement();
    CheckNullPtr_PrintLog_ReturnNullPtr(representation, "Failed to create representation node.", ERROR);
    representation->SetId(xmlRepresentation->GetAttributeVal(INDEX));
    representation->SetCodecs(xmlRepresentation->GetAttributeVal(CODECS));
    representation->SetMimeType(xmlRepresentation->GetAttributeVal(MIMETYPE));
    representation->SetWidth(StringToInt(xmlRepresentation->GetAttributeVal(WIDTH)));
    representation->SetHeight(StringToInt(xmlRepresentation->GetAttributeVal(HEIGHT)));
    representation->SetFrameRate(xmlRepresentation->GetAttributeVal(FRAMERATE));
    representation->SetSar(xmlRepresentation->GetAttributeVal(SAR));
    representation->SetStartWithSAP(xmlRepresentation->GetAttributeVal(STARTWITHSAP));
    representation->SetQualityRanking(xmlRepresentation->GetAttributeVal(QUALITYRANKING));
    representation->SetBandwidth(StringToInt(xmlRepresentation->GetAttributeVal(BANDWIDTH)));
    representation->SetDependencyID(xmlRepresentation->GetAttributeVal(DEPENDENCYID));

    map<string, string> attributes = xmlRepresentation->GetAttributes();
    representation->AddOriginalAttributes(attributes);

    vector<OmafXMLElement*> childElement = xmlRepresentation->GetChildElements();
    for(auto child : childElement)
    {
        if(!child)
        {
            LOG(WARNING)<<"Faild to load sub element in Representation Element."<<endl;
            continue;
        }

        if(child->GetName() == "SegmentTemplate")
        {
            SegmentElement* segment = nullptr;
            segment = BuildSegment(child);
            if(segment)
                representation->SetSegment(segment);
            else
                LOG(WARNING)<<"Fail to add segment."<<endl;
        }
        else
        {
            LOG(INFO)<<"Can't parse element in BuildRepresentation."<<endl;
        }
        representation->AddChildElement(child);
    }

    return representation;
}

SegmentElement* OmafMPDReader::BuildSegment(OmafXMLElement* xmlSegment)
{
    CheckNullPtr_PrintLog_ReturnNullPtr(xmlSegment, "Failed to read segment element.", ERROR);

    SegmentElement* segment = new SegmentElement();
    CheckNullPtr_PrintLog_ReturnNullPtr(segment, "Failed to create segment node.", ERROR);

    segment->SetMedia(xmlSegment->GetAttributeVal(MEDIA));
    segment->SetInitialization(xmlSegment->GetAttributeVal(INITIALIZATION));
    segment->SetDuration(StringToInt(xmlSegment->GetAttributeVal(DURATION)));
    segment->SetStartNumber(StringToInt(xmlSegment->GetAttributeVal(STARTNUMBER)));
    segment->SetTimescale(StringToInt(xmlSegment->GetAttributeVal(TIMESCALE)));

    map<string, string> attributes = xmlSegment->GetAttributes();
    segment->AddOriginalAttributes(attributes);

    vector<OmafXMLElement*> childElement = xmlSegment->GetChildElements();
    for(auto child : childElement)
    {
        if(!child)
        {
            LOG(WARNING)<<"Faild to load sub element in Segment Element."<<endl;
            continue;
        }

        segment->AddChildElement(child);
    }

    return segment;
}

SupplementalPropertyElement* OmafMPDReader::BuildSupplementalProperty(OmafXMLElement* xmlSupplementalProperty)
{
    CheckNullPtr_PrintLog_ReturnNullPtr(xmlSupplementalProperty, "Failed to read Supplemental Property element.", ERROR);

    SupplementalPropertyElement* supplementalProperty = new SupplementalPropertyElement();
    CheckNullPtr_PrintLog_ReturnNullPtr(supplementalProperty, "Failed to create Supplemental Property node.", ERROR);

    supplementalProperty->SetSchemeIdUri(xmlSupplementalProperty->GetAttributeVal(SCHEMEIDURI));
    supplementalProperty->SetValue(xmlSupplementalProperty->GetAttributeVal(VALUE));

    supplementalProperty->ParseSchemeIdUriAndValue();

    map<string, string> attributes = xmlSupplementalProperty->GetAttributes();
    supplementalProperty->AddOriginalAttributes(attributes);

    vector<OmafXMLElement*> childElement = xmlSupplementalProperty->GetChildElements();
    for(auto child : childElement)
    {
        if(!child)
        {
            LOG(WARNING)<<"Faild to load sub element in supplementalProperty Element."<<endl;
            continue;
        }

        if(child->GetName() == OMAF_SPHREGION_QUALITY)
        {
            // suppose supplementalProperty only have 1 SphRegionQuality now
            supplementalProperty->SetSphereRegionQuality(BuildSphRegionQuality(child));
        }
        supplementalProperty->AddChildElement(child);
    }

    return supplementalProperty;
}

SphRegionQualityElement* OmafMPDReader::BuildSphRegionQuality(OmafXMLElement* xmlSphRegionQuality)
{
    CheckNullPtr_PrintLog_ReturnNullPtr(xmlSphRegionQuality, "Failed to read sphere Region Quality element.", ERROR);

    SphRegionQualityElement* sphRegionQuality = new SphRegionQualityElement();
    CheckNullPtr_PrintLog_ReturnNullPtr(sphRegionQuality, "Failed to create sphere Region Quality node.", ERROR);

    sphRegionQuality->SetShapeType(StringToInt(xmlSphRegionQuality->GetAttributeVal(SHAPE_TYPE)));
    sphRegionQuality->SetRemainingAreaFlag((xmlSphRegionQuality->GetAttributeVal(REMAINING_AREA_FLAG) == "true"));
    sphRegionQuality->SetQualityRankingLocalFlag((xmlSphRegionQuality->GetAttributeVal(QUALITY_RANKING_LOCAL_FLAG) == "true"));
    sphRegionQuality->SetQualityType(StringToInt(xmlSphRegionQuality->GetAttributeVal(QUALITY_TYPE)));

    map<string, string> attributes = xmlSphRegionQuality->GetAttributes();
    sphRegionQuality->AddOriginalAttributes(attributes);

    vector<OmafXMLElement*> childElement = xmlSphRegionQuality->GetChildElements();
    for(auto child : childElement)
    {
        if(!child)
        {
            LOG(WARNING)<<"Faild to load sub element in sphRegionQuality Element."<<endl;
            continue;
        }

        if(child->GetName() == OMAF_QUALITY_INFO)
        {
            sphRegionQuality->AddQualityInfo(BuildQualityInfo(child));
        }
        sphRegionQuality->AddChildElement(child);
    }

    return sphRegionQuality;
}

QualityInfoElement* OmafMPDReader::BuildQualityInfo(OmafXMLElement* xmlQualityInfo)
{
    CheckNullPtr_PrintLog_ReturnNullPtr(xmlQualityInfo, "Failed to read Quality Info element.", ERROR);

    QualityInfoElement* qualityInfo = new QualityInfoElement();
    CheckNullPtr_PrintLog_ReturnNullPtr(qualityInfo, "Failed to create Quality Info node.", ERROR);

    qualityInfo->SetAzimuthRange(StringToInt(xmlQualityInfo->GetAttributeVal(AZIMUTH_RANGE)));
    qualityInfo->SetCentreAzimuth(StringToInt(xmlQualityInfo->GetAttributeVal(CENTRE_AZIMUTH)));
    qualityInfo->SetCentreElevation(StringToInt(xmlQualityInfo->GetAttributeVal(CENTRE_ELEVATION)));
    qualityInfo->SetCentreTilt(StringToInt(xmlQualityInfo->GetAttributeVal(CENTRE_TILT)));
    qualityInfo->SetElevationRange(StringToInt(xmlQualityInfo->GetAttributeVal(ELEVATION_RANGE)));
    qualityInfo->SetOrigHeight(StringToInt(xmlQualityInfo->GetAttributeVal(ORIG_HEIGHT)));
    qualityInfo->SetOrigWidth(StringToInt(xmlQualityInfo->GetAttributeVal(ORIG_WIDTH)));
    qualityInfo->SetQualityRanking(StringToInt(xmlQualityInfo->GetAttributeVal(QUALITY_RANKING)));

    map<string, string> attributes = xmlQualityInfo->GetAttributes();
    qualityInfo->AddOriginalAttributes(attributes);

    vector<OmafXMLElement*> childElement = xmlQualityInfo->GetChildElements();
    for(auto child : childElement)
    {
        if(!child)
        {
            LOG(WARNING)<<"Faild to load sub element in qualityInfo Element."<<endl;
            continue;
        }

        qualityInfo->AddChildElement(child);
    }

    return qualityInfo;
}

VCD_OMAF_END
