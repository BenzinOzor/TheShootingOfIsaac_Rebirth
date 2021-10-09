#pragma once


class SoIROptionsEntry
{
public:
	SoIROptionsEntry();
	~SoIROptionsEntry();
	
	int		MoveLeft();
	int		MoveRight();
	int		Validate();

	int		GetMaxValue() const;
	void	SetMaxValue( int _iMax, bool _bLoopValues = false );
	int		GetValue() const;
	void	SetValue( int _iValue );

protected:
	int		m_iMaxValue;
	int		m_iValue;
	bool	m_bLoopValues;
};


class SoIROptions
{
public:
	enum Entry
	{
		eSound,
		eMusic,
		eFullScreen,
		eDamage,
		eNbEntries,
	};

	SoIROptions();
	~SoIROptions();

	int					MoveLeft( const Entry& _eEntry );
	int					MoveRight( const Entry& _eEntry );
	int					Validate( const Entry& _eEntry );

	int					GetMaxValue( const Entry& _eEntry ) const;
	int					GetValue( const Entry& _eEntry ) const;

	void				Save();
	void				ApplyAllSettings();

protected:
	void				_Load();
	void				_ApplySetting( const Entry& _eEntry, int _iValue );

	SoIROptionsEntry	m_pEntries[ Entry::eNbEntries ];
};
