#pragma once

namespace fzn
{
	class CallbackBase;
}

class SoIRFadeManager
{
public:
	SoIRFadeManager();
	~SoIRFadeManager();

	void				Init();

	void				Update();
	void				Display();

	void				FadeToAlpha( float _fInitialAlpha, float _fTargetAlpha, float _fFadeDuration, fzn::CallbackBase* _pCallback = nullptr, bool _bBackToInitialValue = false );

protected:
	float				m_fInitialAlpha;
	float				m_fTargetAlpha;
	float				m_fFadeDuration;
	float				m_fFadeTimer;
	bool				m_bBackToInitialValue;

	sf::RectangleShape	m_oFade;

	fzn::CallbackBase*	m_pCallback;
};


///////////////// CALLBACKS /////////////////

void FctFadeMgrUpdate(void* _data);
void FctFadeMgrDisplay(void* _data);
