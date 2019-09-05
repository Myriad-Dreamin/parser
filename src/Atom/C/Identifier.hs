module Atom.C.Identifier (varName) where

import Text.ParserCombinators.Parsec

varName :: Parser String
varName = do
    (:) <$> firstChar <*> (many restChar)
    where
        firstChar = letter <|> char '_'
        restChar = digit <|> firstChar
    