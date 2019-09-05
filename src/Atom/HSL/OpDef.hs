module Atom.HSL.OpDef (OpDef, opDef) where

import Text.ParserCombinators.Parsec
import Atom.String
import Text.Parsec.Char
import Control.Monad (void)
import Atom.Wrapper

data OpDef = OpDef  { entityType   :: String
                    , identifier :: String
                    , opIntent :: OpIntent
                    } deriving (Show)

data OpIntent = Payment | Invocation deriving (Show)


validName :: Parser String
validName = many1 $ alphaNum <|> oneOf "-_"

opDef :: Parser OpDef
opDef = do
    void $ wrapperIgnoreWhiteSpace $ opDefSpec
    id <- wrapperIgnoreWhiteSpace $ validName
    -- todo

opDefSpec :: Parser String
opDefSpec = string "op"

-- op op1 invocation c1.GetStrikePrice() using a1
-- op op2 payment 50 xcoin from a1 to a2 with 1 xcoin as 0.5 ycoin