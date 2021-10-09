#pragma once

#include "TSOIR/SoIRDefines.h"
#include "TSOIR/Game/SoIRDrawable.h"


class SoIREntity : public SoIRDrawable
{
public:
	struct EntityDesc
	{
		std::string					m_sName				= "";
		SoIREntityPropertiesMask	m_uProperties		= 0;
		SoIRDrawableLayer			m_eLayer			= SoIRDrawableLayer::eNbLayers;
		float						m_fDamage			= 0.f;
		float						m_fHitBoxRadius		= 0.f;
		sf::Vector2f				m_vHitBoxCenter		= { 0.f, 0.f };
		int							m_iHitBoxFirstFrame = -1;
		int							m_iHitBoxLastFrame	= -1;

		std::vector< EntityAnimDesc >		m_oAnimations;
		bool						m_bLoopAnimation	= false;

		std::string					m_sSound		= "";
	};

	SoIREntity( const sf::Vector2f& _vPosition, const EntityDesc* _pDesc, fzn::Anm2::TriggerCallback _pExternalCallback = nullptr, const std::string& _sExternalCallbackName = fzn::Anm2::ANIMATION_END );
	SoIREntity( const sf::Vector2f& _vPosition, const std::string& _sEntity, fzn::Anm2::TriggerCallback _pExternalCallback = nullptr, const std::string& _sExternalCallbackName = fzn::Anm2::ANIMATION_END );
	~SoIREntity();

	void							Update();
	void							Display();
	virtual void					Draw( const SoIRDrawableLayer& _eLayer ) override;

	std::string						GetName() const;
	virtual sf::Vector2f			GetPosition() const override;
	void							SetPosition( const sf::Vector2f& _vPosition );
	static bool						MustBeRemoved( const SoIREntity* _pEntity );
	bool							HasProperty( const SoIREntityProperty& _eProperty ) const;
	void							AddProperty( const SoIREntityProperty& _eProperty );
	void							RemoveProperty( const SoIREntityProperty& _eProperty );
	void							SetColorOverlay( const sf::Glsl::Vec4 _oColor );

protected:
	SoIREntity();

	void							_LoadFromDesc( const EntityDesc* _pDesc, fzn::Anm2::TriggerCallback _pExternalCallback = nullptr, const std::string& _sExternalCallbackName = fzn::Anm2::ANIMATION_END );

	virtual void					_OnAnimationEvent( std::string _sEvent, const fzn::Anm2* _pAnim );

	fzn::Anm2						m_oAnim;
	std::vector< AnimTriggerDesc >	m_oAnimTriggers;
	sf::CircleShape*				m_pHitBox;
	bool							m_bUseHitBox;

	std::string						m_sName;
	SoIRDrawableLayer				m_eLayer;
	SoIREntityPropertiesMask		m_uProperties;
	sf::Vector2f					m_vPosition;
	float							m_fDamage;

	sf::Shader*						m_pShader;
	sf::Glsl::Vec4					m_oColorOverlay;

	fzn::Anm2::TriggerCallback		m_pAnimCallback;
};
