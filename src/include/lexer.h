#ifndef LEXER_H
#define LEXER_H
typedef enum{
  TOK_KEYWORD = 0,
  TOK_NUM,
  TOK_SYMBOL,
  TOK_PLUS,
  TOK_MINUS,
  TOK_STAR,
  TOK_SLASH,
  TOK_OPAREN,
  TOK_CPAREN,
  TOK_SCOLON,
  TOK_EQUAL,
  TOK_AT,
  TOK_CARROT,
  TOK_EOF,
  __tok_type__end
}tok_type;

#ifdef DEBUG
static char tok_type_str[][50] = {
  "TOK_KEYWORD",
  "TOK_NUM",
  "TOK_SYMBOL",
  "TOK_PLUS",
  "TOK_MINUS",
  "TOK_STAR",
  "TOK_SLASH",
  "TOK_OPAREN",
  "TOK_CPAREN",
  "TOK_SCOLON",
  "TOK_AT",
  "TOK_CARROT",
  "TOK_EQUAL",
  "TOK_EOF"
};
#endif
enum keywordtypes{
  TO_IND = 0, 
  IF_IND,
  END_IND,
  FOR_IND,
  VAR_IND,
  BEGIN_IND,
  RETURN_IND,
  FUNCTION_IND,
  __keywords_end
};

typedef struct
{
  char *raw;
  char *filename;
  size_t len;
  tok_type type;
  size_t val;
  size_t line;
  size_t col;
}token_t;

typedef struct{
  token_t *buffer;
  size_t cap;
  size_t len;
}token_slice;
token_slice lex_tokens(char *input, size_t input_len,char *filename);
#define print_token(tok) \
  printf("token_t{\n raw: %.*s\n len: %d\n tok_type: %s\n val %d\n}\n",(tok).len,(tok).raw,(tok).len,tok_type_str[(tok).type],(tok).val);
#endif
