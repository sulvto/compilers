package lexer;

import java.io.IOException;
import java.io.InputStream;
import java.util.Hashtable;

/**
 * Created by sulvto on 17-10-26.
 */
public class Lexer {
    private int line = 1;
    private int column = 1;

    char peek = ' ';
    Hashtable<String, Word> wordTable = new Hashtable<>();
    private InputStream inputStream;

    public Lexer(InputStream inputStream) {
        this.inputStream = inputStream;
        reserve(newWord("if", Tag.IF));
        reserve(newWord("return", Tag.RETURN));
        reserve(newWord("print", Tag.BREAK));
        reserve(newWord("while", Tag.WHILE));
        reserve(newWord("struct", Tag.STRUCT));
    }


    private void reserve(Word word) {
        wordTable.put(word.lexeme, word);
    }


    private void readch() {
        try {
            peek = (char) inputStream.read();
        } catch (IOException e) {
            throw new Error(e.getMessage());
        }
        column++;
    }

    private boolean readch(char c) {
        readch();
        if (peek == c) {
            peek = ' ';
            return true;
        } else {
            return false;
        }
    }


    private Token newToken(int tag) {
        return new Token(tag, line, column);
    }

    private Word newWord(String lexeme, int tag) {
        return new Word(lexeme, tag, line, column);
    }

    public Token nextToken() {
        if (end()) return newToken(Tag.EOF);

        for (; ; readch()) {
            if (peek == ' ' || peek == '\t') ;
            else if (peek == '\n') {
                column = 1;
                line++;
                if (end()) return newToken(Tag.EOF);
            } else break;
        }

        if (end()) return newToken(Tag.EOF);

        //
        /**
         *
         */
        if (peek == '/') {
            readch();
            if (peek == '/') {
                for (; ; readch()) {
                    if (peek == '\n') {
                        line++;
                        column = 1;
                        readch();
                        return nextToken();
                    }
                }
            } else if (peek == '*') {
                readch();
                for (; ; readch()) {
                    if (peek == '*' && readch('/')) {
                        return nextToken();
                    } else if (peek == '\n') {
                        line++;
                        column = 1;
                    } else {
                        // TODO eof
                    }
                }
            } else {
                return newToken('/');
            }
        }

        int line = this.line;
        int column = this.column;

        switch (peek) {
            case '&':
                 return newToken('&');
            case '|':
                 return newToken('|');
            case '!':
                return newToken('!');
            case '=':
                return newToken('=');
            case '>':
                return newToken('<');
            case '<':
                return newToken('<');
        }

        // "string"
        if (peek == '\"') {
            StringBuilder buf = new StringBuilder();
            do {
                readch();
                buf.append(peek);
            } while (peek != '\"');
            readch();
            return newWord(buf.toString(), Tag.STRING);
        }

        // TODO Tag.CHARACTER

        if (Character.isDigit(peek)) {
            int v = 0;
            do {
                v = 10 * v + Character.digit(peek, 10);
                readch();
            } while (Character.isDigit(peek));

            if ('.' != peek) {
                return new Num(v,line,column);
            }

            float x = v;
            float d = 10;
            for (; ; ) {
                readch();
                if (!Character.isDigit(peek)) break;
                x = x + Character.digit(peek, 10) / d;
                d *= 10;
            }
            return new Real(x,line,column);
        }

        if (Character.isLetter(peek)) {
            StringBuilder buf = new StringBuilder();
            do {
                buf.append(peek);
                readch();
            } while (Character.isLetterOrDigit(peek));

            String t = buf.toString();
            Word word = wordTable.get(t);
            if (word == null) {
                word = newWord(t, Tag.ID);
                wordTable.put(t, word);
            }
            word.setLine(line);
            word.setColumn(column);
            return word;
        }
        Token token = newToken(peek);
        peek = ' ';
        return token;
    }

    private boolean end() {
        try {
            return inputStream.available() == 0;
        } catch (IOException e) {
            throw new Error(e.getMessage());
        }
    }
}
