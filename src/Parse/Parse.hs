module Parse.Parse  (prpr, prToEnd, prToAny) where


import Text.ParserCombinators.Parsec
prpr :: Parser a -> String -> Either ParseError a
prpr a = parse a "QAQ"

prToEnd :: Parser a -> String -> Either ParseError a
prToEnd a = parse (a <* eof) "QAQ"

prToAny :: Parser a -> String -> Either ParseError (a,String)
prToAny p = parse ((,) <$> p <*> leftOver) "QAQ"
  where leftOver = manyTill anyToken eof
