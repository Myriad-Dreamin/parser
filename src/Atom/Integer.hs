module Atom.Integer (bigNumber) where

import Text.Parsec.String
import Text.Parsec.Char (digit)
import Text.Parsec.Combinator (many1)
import Text.ParserCombinators.Parsec

bigNumber :: Parser Integer

bigNumber = do
    n <- many1 digit
    return (read n)

