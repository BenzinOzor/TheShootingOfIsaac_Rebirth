#pragma once

#include <vector>

#include <FZN/Display/BitmapText.h>
#include <FZN/Display/ProgressBar.h>

#include "TSOIR/Game/SoIRDrawable.h"


class SoIRSpiderMod : public SoIRDrawable
{
public:
	SoIRSpiderMod();
	~SoIRSpiderMod();

	void							Update();
	void							OnEvent();
	void							Display();
	virtual void					Draw( const SoIRDrawableLayer& _eLayer ) override;

	void							OnHide();

protected:
	struct DamageTextInfo
	{
		fzn::BitmapText m_oText;
		float			m_fTimer		= 0.f;
		sf::Vector2f	m_vInitialPos	= { 0.f, 0.f };
		sf::Vector2f	m_vFinalPos		= { 0.f, 0.f };
	};

	fzn::ProgressBar				m_oHealthBar;
	std::vector< DamageTextInfo >	m_oDamageTexts;

	static constexpr float			DAMANGE_TEXT_DURATION	= 1.f;
	static constexpr float			VERTICAL_OFFSET			= 60.f;
};


///////////////// CALLBACKS /////////////////

void FctSpiderModEvent( void* _pData );
