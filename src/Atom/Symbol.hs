-- module Atom.Symbol (parens) where

-- -- import Text.ParserCombinators.Parsec
-- -- import Control.Monad (void)
-- -- import Atom.Wrapper

-- -- parens :: Parser a -> Parser a
-- parens parser = do 
--     void $ wrapperIgnoreWhiteSpace $ char '('
--     e <- parser
--     void $ wrapperIgnoreWhiteSpace $ char ')'
--     return $ e
