#pragma once

#include <FZN/Display/BitmapText.h>

#include "TSOIR/Menus/SoIRBaseMenu.h"

class SoIRSaveScoreMenu : public SoIRBaseMenu
{
public:
	SoIRSaveScoreMenu( const sf::Vector2f& _vPosition );
	~SoIRSaveScoreMenu();

	virtual void			Draw( const SoIRDrawableLayer& _eLayer ) override;
	void					OnEvent();
	
	virtual void			OnPush( const SoIRMenuID& _ePreviousMenuID ) override;
	
	virtual void			MoveUp() override;
	virtual void			MoveDown() override;
	virtual void			MoveLeft() override;
	virtual void			MoveRight() override;
	virtual void			Validate() override;
	virtual void			Back() override;

protected:
	bool					_IsAuthorizedCharacter( char _cChar ) const;
	void					_IncreaseCharacter();
	void					_DecreaseCharacter();

	fzn::BitmapText			m_oTitle;

	fzn::BitmapText			m_oLevel;
	fzn::BitmapText			m_oScoreHeader;
	fzn::BitmapText			m_oScore;
	fzn::BitmapText			m_oNameHeader;
	fzn::BitmapText			m_oName;

	int						m_iCurrentLetter;
	static constexpr int	LETTERS_NUMBER = 3;
};

void FctSaveScoreMenuEvent( void* _pData );
