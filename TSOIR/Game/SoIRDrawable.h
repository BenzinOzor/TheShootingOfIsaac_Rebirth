#pragma once

#include "TSOIR/SoIRDefines.h"


class TSOIR_EXPORT SoIRDrawable
{
public:
	SoIRDrawable() : m_eGameElementType( SoIRGameElementType::eNbGameElementTypes ) {}
	~SoIRDrawable() {}

	virtual void Draw( const SoIRDrawableLayer& _eLayer ) = 0;

	virtual sf::Vector2f	GetPosition() const { return { 0.f, 0.f }; }

	SoIRGameElementType GetGameElementType() const { return m_eGameElementType; }

protected:
	SoIRGameElementType m_eGameElementType;
};
