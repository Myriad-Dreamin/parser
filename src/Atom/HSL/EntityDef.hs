module Atom.HSL.EntityDef (EntityDef, entityDef) where

import Text.ParserCombinators.Parsec
import Atom.String
import Text.Parsec.Char
import Control.Monad (void)
import Atom.Wrapper

data EntityDef = EntityDef  { entityType   :: String
                            , identifier :: String
                            , chainName    :: String
                            , constructor :: Constructor
                            } deriving (Show)

data Constructor = Constructor  { constructType :: String
                                , address       :: String
                                , token         :: String
                                , unit          :: String
                                } deriving (Show)


validName :: Parser String
validName = many1 $ alphaNum <|> oneOf "-_"

entityDef :: Parser EntityDef
entityDef = do
    et <- wrapperIgnoreWhiteSpace $ entityTypeParser
    id <- wrapperIgnoreWhiteSpace $ validName
    void $ wrapperIgnoreWhiteSpace $ char '='
    cn <- wrapperIgnoreWhiteSpace $ chainNameParser
    void $ string "::"
    ct <- constructorParser
    return $ EntityDef et id cn ct

entityTypeParser :: Parser String
entityTypeParser = string "account" <|> string "contract"

chainNameParser :: Parser String
chainNameParser = do
    void $ string "Chain"
    validName

constructorParser :: Parser Constructor
constructorParser = do
    ct <- validName
    void $ wrapperIgnoreWhiteSpace $ char '('
    void $ wrapperIgnoreWhiteSpace $ string "0x"
    address <- lowHexString
    void $ wrapperIgnoreWhiteSpace $ char ','
    token <- wrapperIgnoreWhiteSpace $ many1 digit
    void $ wrapperIgnoreWhiteSpace $ char ','
    unit <- wrapperIgnoreWhiteSpace $ many1 letter
    void $ wrapperIgnoreWhiteSpace $ char ')'
    return $ Constructor ct address token unit