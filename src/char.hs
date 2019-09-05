import Text.Parsec.String
import Text.Parsec.Char
import Text.ParserCombinators.Parsec

character :: Char -> Parser Char
character = satisfy . (==)

bind :: Parser String
bind = do
  character '>'
  character '>'
  character '='
  return ">>="


  