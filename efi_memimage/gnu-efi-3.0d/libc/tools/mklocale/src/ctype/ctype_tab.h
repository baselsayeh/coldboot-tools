typedef union	{
    rune_t	rune;
    int		i;
    char	*str;

    rune_list	*list;
} YYSTYPE;
#define	RUNE	258
#define	LBRK	259
#define	RBRK	260
#define	THRU	261
#define	MAPLOWER	262
#define	MAPUPPER	263
#define	DIGITMAP	264
#define	LIST	265
#define	VARIABLE	266
#define	ENCODING	267
#define	INVALID	268
#define	STRING	269


extern YYSTYPE yylval;
