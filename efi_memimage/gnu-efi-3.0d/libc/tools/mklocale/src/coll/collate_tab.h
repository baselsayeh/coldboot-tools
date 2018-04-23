typedef union {
	u_char ch;
	u_char str[STR_LEN];
} YYSTYPE;
#define	SUBSTITUTE	258
#define	WITH	259
#define	ORDER	260
#define	RANGE	261
#define	STRING	262
#define	CHAIN	263
#define	DEFN	264
#define	CHAR	265


extern YYSTYPE yylval;
