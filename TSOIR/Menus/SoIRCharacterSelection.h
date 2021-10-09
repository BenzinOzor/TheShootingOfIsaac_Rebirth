#pragma once

#include "TSOIR/Menus/SoIRBaseMenu.h"


class SoIRCharacterSelection : public SoIRBaseMenu
{
public: 
	explicit SoIRCharacterSelection( const sf::Vector2f& _vPosition );
	virtual ~SoIRCharacterSelection();
	
	virtual void					Update() override;
	virtual void Draw( const SoIRDrawableLayer& _eLayer ) override;
	
	virtual void					OnPush( const SoIRMenuID& _ePreviousMenuID ) override;

	virtual void					MoveLeft() override;
	virtual void					MoveRight() override;
	virtual void					Validate() override;

	SoIRCharacter					GetSelectedCharacter() const;

	virtual void					DrawImGUI() override;

protected:
	struct SlotInfo
	{
		SlotInfo()
			: m_vPosition( 0.f, 0.f )
			, m_fEllipseAngle( 0.f )
			, m_fRatio( -1.f )
		{}

		bool IsValid() const
		{
			return m_fRatio >= 0.f;
		}

		sf::Vector2f m_vPosition;
		float m_fEllipseAngle;
		float m_fRatio;
	};

	struct CharacterInfo
	{
		SoIRCharacter	m_eCharacter;
		int				m_iCurrentSlot;
		sf::Vector2f	m_vCurrentPosition;
		float			m_fCurrentRatio;
		int				m_iInitialSlot;
		float			m_fFinalAngle;
	};

	void							_InitCharacters();
	void							_UpdateMenuAnim();
	void							_UpdateScrolling();

	void							_DisplayDebugEllipse();

	static bool						_CharacterDrawSorter( const CharacterInfo* _CharA, const CharacterInfo* _CharB );

	fzn::Anm2						m_oCharacters;

	std::string						m_pAnimNames[ SoIRCharacter::eNbCharacters ];
	SlotInfo						m_pCharactersSlots[ SoIRCharacter::eNbCharacters ];
	CharacterInfo					m_pCharacters[ SoIRCharacter::eNbCharacters ];
	std::vector< CharacterInfo* >	m_oSortedCharacters;

	sf::Vector2f					m_vEllipseCenter;
	float							m_fEllipseRadiusX;
	float							m_fEllipseRadiusY;
	bool							m_bScrolling;
	float							m_fScrollDuration;
	float							m_fScrollTimer;
	SoIRCharacter					m_eRandomCharacter;
	int								m_iNbMoveRights;
	int								m_iMaxMoveRights;
	bool							m_bRandomSelectionSlowDown;
};
