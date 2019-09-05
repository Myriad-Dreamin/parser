module Atom.HSL.ImportSeg (ImportSeg, importSeg) where

import Text.ParserCombinators.Parsec
import Text.Parsec.Char
import Control.Monad (void)
import Atom.Wrapper

data ImportSeg = ImportSpec [FileName] deriving (Show)
type FileName = String

importSeg :: Parser ImportSeg
importSeg = ImportSpec <$> (wrapperIgnoreWhiteSpace $ importSpec)



importSpec :: Parser [FileName]
importSpec = do
    void $ wrapperIgnoreWhiteSpace $ string "import"
    wrapperIgnoreWhiteSpace $ between (char '(') (char ')') decodeImportFiles

decodeImportFiles :: Parser [FileName]
decodeImportFiles = do
    f <- wrapperIgnoreWhiteSpace $ decodeImportFile
    mayBeMoreFiles [f]
    where
    decodeImportFile = char '"' *> (manyTill anyChar $ char '"')
    moreFiles fs = do
        void $ wrapperIgnoreWhiteSpace $ char ','
        f <- wrapperIgnoreWhiteSpace $ decodeImportFile
        mayBeMoreFiles $ f : fs
    mayBeMoreFiles fs = moreFiles fs <|> return fs
