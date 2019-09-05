module Wrapper (ignoreWhiteSpace, wrapperIgnoreWhiteSpace) where

import Text.ParserCombinators.Parsec
import Control.Monad (void)

ignoreWhiteSpace :: Parser ()
ignoreWhiteSpace = void $ many $ oneOf "\n\t "

wrapperIgnoreWhiteSpace :: Parser a -> Parser a
wrapperIgnoreWhiteSpace = (>>) ignoreWhiteSpace
