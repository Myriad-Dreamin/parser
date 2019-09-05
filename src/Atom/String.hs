module Atom.String (lowHexString) where

import Text.Parsec.Char (digit)
import Text.Parsec.Combinator (many1)
import Text.ParserCombinators.Parsec

lowHexString :: Parser String
lowHexString = many1 $ digit <|> oneOf "abcdef"

