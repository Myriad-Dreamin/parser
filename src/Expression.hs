module Expression (exprize) where

import Text.Parsec.String
import Text.Parsec.Char
-- import Control.Applicative ((<|>))
import Text.ParserCombinators.Parsec
import Control.Monad (void)
import Data.Char (isLetter, isDigit)
import qualified Atom.Integer
import Atom.C.Identifier
-- import qualified Atom.Symbol
import Atom.Wrapper

data Expr = Num Integer | Var String | Parentheses Expr | BiOperator String Expr Expr deriving (Eq,Show)

var :: Parser Expr
var = Var <$> (wrapperIgnoreWhiteSpace $ varName)

num :: Parser Expr
num = Num <$> (wrapperIgnoreWhiteSpace $ Atom.Integer.bigNumber)

vnn :: Parser Expr
vnn = var <|> num

parens :: Parser Expr -> Parser Expr
parens parser = do
    void $ wrapperIgnoreWhiteSpace $ char '('
    e <- parser
    void $ wrapperIgnoreWhiteSpace $ char ')'
    return $ Parentheses e
    

operator :: Parser String   
operator = choice (fmap (try . wrapperIgnoreWhiteSpace . string) ["+", "-"])

applyOpeator :: Parser Expr -> Parser Expr -> Parser Expr
applyOpeator parserA parserB = do
    e <- wrapperIgnoreWhiteSpace $ parserA
    maybeEnd e
    where
    notEnd e = do
        o <- operator
        b <- wrapperIgnoreWhiteSpace $ parserB
        maybeEnd $ BiOperator o e b
    maybeEnd e = notEnd e <|> return e

exprize :: Parser Expr
exprize = try (applyOpeator baseExpr baseExpr) <|> item
baseExpr :: Parser Expr
baseExpr = try (applyOpeator item item) <|> item
item :: Parser Expr
item = try vnn <|> parens exprize
-- prpr exprize " a + ( b + c ) + d "
-- prpr exprize " a + b + c + d "