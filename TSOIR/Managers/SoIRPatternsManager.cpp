#include <tinyXML2/tinyxml2.h>

#include "TSOIR/Managers/SoIRPatternsManager.h"


const char* PATTERNS_FILE_NAME	= "XMLFiles/Patterns.cfg";

SoIRPatternsManager::SoIRPatternsManager()
{
}

SoIRPatternsManager::~SoIRPatternsManager()
{
}

void SoIRPatternsManager::Init()
{
	LoadPatternsFile( DATAPATH( PATTERNS_FILE_NAME ), m_oPatterns, m_oGroups );
}

void SoIRPatternsManager::LoadPatternsFile( const std::string& _sFile, std::vector< SoIRPattern::Desc >& _oPatterns, std::vector< SoIRPattern::Group >& _oGroups )
{
	tinyxml2::XMLDocument resFile;

	if( g_pFZN_DataMgr->LoadXMLFile( resFile, _sFile ) )
	{
		FZN_COLOR_LOG( fzn::DBG_MSG_COL_RED, "Failure : %s.", resFile.ErrorName() );
		return;
	}

	tinyxml2::XMLElement* pParentElement = resFile.FirstChildElement( "Patterns" );

	if( pParentElement == nullptr )
	{
		FZN_COLOR_LOG( fzn::DBG_MSG_COL_RED, "Failure : \"Patterns\" tag not found." );
		return;
	}

	tinyxml2::XMLElement* pElement = pParentElement->FirstChildElement( "Pattern" );

	if( pElement == nullptr )
	{
		FZN_COLOR_LOG( fzn::DBG_MSG_COL_RED, "Failure : \"Pattern\" tag not found." );
		return;
	}

	while( pElement != nullptr )
	{
		SoIRPattern::Desc oDesc;
		
		oDesc.m_sName = fzn::Tools::XMLStringAttribute( pElement, "Name" );

		_LoadProjectilesSettings( oDesc, pElement->FirstChildElement( "ProjectilesSettings" ) );
		_LoadPatternSettings( oDesc, pElement->FirstChildElement( "PatternSettings" ) );

		if( oDesc.IsValid() )
			_oPatterns.push_back( oDesc );

		pElement = pElement->NextSiblingElement();
	}

	pParentElement = resFile.FirstChildElement( "Groups" );

	if( pParentElement == nullptr )
	{
		FZN_COLOR_LOG( fzn::DBG_MSG_COL_RED, "Failure : \"Groups\" tag not found." );
		return;
	}

	pElement = pParentElement->FirstChildElement( "Group" );

	if( pElement == nullptr )
	{
		FZN_COLOR_LOG( fzn::DBG_MSG_COL_RED, "Failure : \"Pattern\" tag not found." );
		return;
	}

	while( pElement != nullptr )
	{
		SoIRPattern::Group oGroup;

		oGroup.m_sName = fzn::Tools::XMLStringAttribute( pElement, "Name" );

		_LoadPatternGroup( oGroup, pElement, _oPatterns );

		if( oGroup.m_oPatterns.empty() == false )
			_oGroups.push_back( oGroup );

		pElement = pElement->NextSiblingElement();
	}
}

const SoIRPattern::Desc* SoIRPatternsManager::GetPattern( const std::string& _sPattern ) const
{
	if( _sPattern.empty() )
		return nullptr;

	std::vector< SoIRPattern::Desc >::const_iterator it = std::find_if( m_oPatterns.begin(), m_oPatterns.end(), [ _sPattern ]( const SoIRPattern::Desc& _oDesc ) { return _oDesc.m_sName == _sPattern; } );

	if( it == m_oPatterns.end() )
		return nullptr;

	return &(*it);
}

const SoIRPattern::Group* SoIRPatternsManager::GetGroup( const std::string& _sGroup ) const
{
	if( _sGroup.empty() )
		return nullptr;

	std::vector< SoIRPattern::Group >::const_iterator it = std::find_if( m_oGroups.begin(), m_oGroups.end(), [ _sGroup ]( const SoIRPattern::Group& _oGroup ) { return _oGroup.m_sName == _sGroup; } );

	if( it == m_oGroups.end() )
		return nullptr;

	return &(*it);
}

const SoIRPattern::Desc* SoIRPatternsManager::GetRandomPatternFromGroup( const std::string& _sGroup ) const
{
	if( _sGroup.empty() )
		return nullptr;

	std::vector< SoIRPattern::Group >::const_iterator it = std::find_if( m_oGroups.begin(), m_oGroups.end(), [_sGroup]( const SoIRPattern::Group& _oGroup ) { return _oGroup.m_sName == _sGroup; } );

	if( it == m_oGroups.end() || (*it).m_oPatterns.empty() )
		return nullptr;

	const int iRandomPattern = Rand( 0, (*it).m_oPatterns.size() );

	const std::string sPattern = (*it).m_oPatterns[ iRandomPattern ];

	return GetPattern( sPattern );
}

void SoIRPatternsManager::_LoadProjectilesSettings( SoIRPattern::Desc& _oDesc, tinyxml2::XMLElement* _pElement )
{
	if( _pElement == nullptr )
		return;

	int iValue = 0;
	bool bValue = false;
	unsigned int uValue = 0;
	float fValue = 0.f;

	tinyxml2::XMLError eError = _pElement->QueryIntAttribute( "Type", &iValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		_oDesc.m_eType = (SoIRProjectileType)iValue;


	eError = _pElement->QueryIntAttribute( "ProjectilesPattern", &iValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		_oDesc.m_ePattern = (SoIRProjectilePattern)iValue;


	eError = _pElement->QueryUnsignedAttribute( "Properties", &uValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		_oDesc.m_uProperties = (sf::Uint16)uValue;


	eError = _pElement->QueryBoolAttribute( "CircleRandomAngle", &bValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		_oDesc.m_bCirclePatternRandomAngle = bValue;


	eError = _pElement->QueryFloatAttribute( "SpreadAngle", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		_oDesc.m_fSpreadAngle = fValue;


	eError = _pElement->QueryIntAttribute( "Number", &iValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		_oDesc.m_iNumber = iValue;


	eError = _pElement->QueryFloatAttribute( "HomingRadius", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		_oDesc.m_fHomingRadius = fValue;


	eError = _pElement->QueryFloatAttribute( "Damage", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		_oDesc.m_fDamage = fValue;
}

void SoIRPatternsManager::_LoadPatternSettings( SoIRPattern::Desc& _oDesc, tinyxml2::XMLElement* _pElement )
{
	if( _pElement == nullptr )
		return;

	bool bValue = false;
	float fValue = 0.f;
	int iValue = 0;

	tinyxml2::XMLError eError = _pElement->QueryFloatAttribute( "Duration", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		_oDesc.m_fDuration = fValue;


	eError = _pElement->QueryFloatAttribute( "TimeBeforeRotation", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		_oDesc.m_fTimeBeforeRotation = fValue;


	eError = _pElement->QueryFloatAttribute( "TimeAfterRotation", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		_oDesc.m_fTimeAfterRotation = fValue;
	

	eError = _pElement->QueryFloatAttribute( "AngleStep", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		_oDesc.m_fAngleStep = fValue;


	eError = _pElement->QueryFloatAttribute( "InitialAngle", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		_oDesc.m_fInitialAngle = fValue;


	eError = _pElement->QueryFloatAttribute( "FinalAngle", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		_oDesc.m_fFinalAngle = fValue;
	else
		_oDesc.m_fFinalAngle = _oDesc.m_fInitialAngle;


	eError = _pElement->QueryFloatAttribute( "RotationDirectionDuration", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		_oDesc.m_fRotationDirectionDuration = fValue;


	eError = _pElement->QueryBoolAttribute( "RotationClosestToPlayer", &bValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		_oDesc.m_bRotationClosestToPlayer = bValue;


	eError = _pElement->QueryBoolAttribute( "TargetPlayer", &bValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		_oDesc.m_bTargetPlayer = bValue;


	eError = _pElement->QueryIntAttribute( "FollowPlayerRotation", &iValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		_oDesc.m_iFollowPlayerRotation = iValue;


	eError = _pElement->QueryFloatAttribute( "ShotSpeed", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		_oDesc.m_fShotSpeed = fValue;


	eError = _pElement->QueryFloatAttribute( "TearDelay", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		_oDesc.m_fTearDelay = fValue;


	eError = _pElement->QueryFloatAttribute( "PhaseShift", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		_oDesc.m_oSinParams.m_fPhaseShift = fValue;


	eError = _pElement->QueryFloatAttribute( "MinSpeed", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		_oDesc.m_oSinParams.m_fMinSpeed = fValue;


	eError = _pElement->QueryFloatAttribute( "MaxSpeed", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		_oDesc.m_oSinParams.m_fMaxSpeed = fValue;


	eError = _pElement->QueryFloatAttribute( "Frequency", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		_oDesc.m_oSinParams.m_fFrequency = fValue;


	eError = _pElement->QueryFloatAttribute( "MinSpeedClamp", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		_oDesc.m_oSinParams.m_fMinSpeedClamp = fValue;
	else
		_oDesc.m_oSinParams.m_fMinSpeedClamp = _oDesc.m_oSinParams.m_fMinSpeed;


	eError = _pElement->QueryFloatAttribute( "MaxSpeedClamp", &fValue );

	if( eError != tinyxml2::XML_NO_ATTRIBUTE )
		_oDesc.m_oSinParams.m_fMaxSpeedClamp = fValue;
	else
		_oDesc.m_oSinParams.m_fMaxSpeedClamp = _oDesc.m_oSinParams.m_fMaxSpeed;
}

void SoIRPatternsManager::_LoadPatternGroup( SoIRPattern::Group& _oGroup, tinyxml2::XMLElement* _pElement, std::vector< SoIRPattern::Desc >& _oPatterns )
{
	if( _pElement == nullptr )
		return;

	tinyxml2::XMLElement* pPattern = _pElement->FirstChildElement( "Pattern" );

	while( pPattern != nullptr )
	{
		std::string sPattern = fzn::Tools::XMLStringAttribute( pPattern, "Name" );

		std::vector< SoIRPattern::Desc >::iterator it = std::find_if( _oPatterns.begin(), _oPatterns.end(), [ sPattern ]( const SoIRPattern::Desc& _oDesc ) { return _oDesc.m_sName == sPattern; } );

		if( it != _oPatterns.end() )
			_oGroup.m_oPatterns.push_back( sPattern );

		pPattern = pPattern->NextSiblingElement();
	}
}
