#pragma once

#include <string>
#include <vector>

#include "TSOIR/Game/Enemies/Patterns/SoIRPattern.h"


class TSOIR_EXPORT SoIRPatternsManager
{
public:
	SoIRPatternsManager();
	~SoIRPatternsManager();

	void						Init();

	static void					LoadPatternsFile( const std::string& _sFile, std::vector< SoIRPattern::Desc >& _oPatterns, std::vector< SoIRPattern::Group >& _oGroups );

	const SoIRPattern::Desc*	GetPattern( const std::string& _sPattern ) const;
	const SoIRPattern::Group*	GetGroup( const std::string& _sGroup ) const;
	const SoIRPattern::Desc*	GetRandomPatternFromGroup( const std::string& _sGroup ) const;

protected:
	static void					_LoadProjectilesSettings( SoIRPattern::Desc& _oDesc, tinyxml2::XMLElement* _pElement );
	static void					_LoadPatternSettings( SoIRPattern::Desc& _oDesc, tinyxml2::XMLElement* _pElement );
	static void					_LoadPatternGroup( SoIRPattern::Group& _oGroup, tinyxml2::XMLElement* _pElement, std::vector< SoIRPattern::Desc >& _oPatterns );

	std::vector< SoIRPattern::Desc > m_oPatterns;
	std::vector< SoIRPattern::Group > m_oGroups;
};
