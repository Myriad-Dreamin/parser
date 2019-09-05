module Atom.HSL.Program (Program) where

import Text.ParserCombinators.Parsec

data Program = Program  { importSeg :: ImportSeg
                        , entityDef :: EntityDef
                        , opDef     :: OpDef
                        , depDef    :: DepDef
                        } deriving (Show)


varName :: Parser String
varName = do
    (:) <$> firstChar <*> (many restChar)
    where
        firstChar = letter <|> char '_'
        restChar = digit <|> firstChar
    