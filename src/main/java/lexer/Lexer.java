package lexer;

import java.io.IOException;
import java.io.InputStream;
import java.util.Hashtable;

/**
 * Created by sulvto on 16-12-8.
 */
public class Lexer {
    private int line = 1;
    private int column = 0;

    char peek = ' ';
    Hashtable<String, Word> wordTable = new Hashtable<>();
    private InputStream inputStream;

    public Lexer(InputStream inputStream) {
        this.inputStream = inputStream;

        reserve(new Word("true", Tag.TRUE));
        reserve(new Word("false", Tag.FALSE));
        reserve(new Word("void", Tag.VOID));
        reserve(new Word("char", Tag.CHAR));
        reserve(new Word("long", Tag.LONG));
        reserve(new Word("int", Tag.INT));
        reserve(new Word("float", Tag.FLOAT));
        reserve(new Word("bool", Tag.BOOL));
        reserve(new Word("union", Tag.UNION));
        reserve(new Word("enum", Tag.ENUM));
        reserve(new Word("static", Tag.STATIC));
        reserve(new Word("extern", Tag.EXTERN));
        reserve(new Word("const", Tag.CONST));
        reserve(new Word("if", Tag.IF));
        reserve(new Word("else", Tag.ELSE));
        reserve(new Word("switch", Tag.SWITCH));
        reserve(new Word("case", Tag.CASE));
        reserve(new Word("default", Tag.DEFAULT));
        reserve(new Word("while", Tag.WHILE));
        reserve(new Word("do", Tag.DO));
        reserve(new Word("for", Tag.FOR));
        reserve(new Word("return", Tag.RETURN));
        reserve(new Word("break", Tag.BREAK));
        reserve(new Word("continue", Tag.CONTINUE));
        reserve(new Word("goto", Tag.GOTO));
        reserve(new Word("typedef", Tag.TYPEDEF));
        reserve(new Word("struct", Tag.STRUCT));
        reserve(new Word("import", Tag.IMPORT));
        reserve(new Word("sizeof", Tag.SIZEOF));
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

    public Token nextToken() {
        for (; ; readch()) {
            if (peek == ' ' || peek == '\t') ;
            else if (peek == '\n') {
                column = 0;
                line++;
            } else break;
        }

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
                        column = 0;
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
                        column = 0;
                    }
                }
            }
        }

        int l = line;
        int c = column;

        switch (peek) {
            case '&':
                if (readch('&')) return new Word("&&", Tag.AND, l, c);
                else return new Token('&', l, c);
            case '|':
                if (readch('|')) return new Word("&&", Tag.OR, l, c);
                else return new Token('|', l, c);
            case '!':
                if (readch('=')) return new Word("&&", Tag.NE, l, c);
                else return new Token('!', l, c);
            case '=':
                if (readch('=')) return new Word("&&", Tag.EQ, l, c);
                else return new Token('=', l, c);
            case '>':
                if (readch('=')) return new Word("&&", Tag.GE, l, c);
                else return new Token('<', l, c);
            case '<':
                if (readch('=')) return new Word("&&", Tag.LE, l, c);
                else return new Token('<', l, c);
        }

        // "string"
        if (peek == '\"') {
            StringBuilder buf = new StringBuilder();
            do {
                readch();
                buf.append(peek);
            } while (peek != '\"');
            readch();
            return new Word(buf.toString(), Tag.STRING, l, c);
        }

        // TODO Tag.CHARACTER

        if (Character.isDigit(peek)) {
            int v = 0;
            do {
                v = 10 * v + Character.digit(peek, 10);
                readch();
            } while (Character.isDigit(peek));

            if ('.' != peek) {
                return new Num(v, l, c);
            }

            float x = v;
            float d = 10;
            for (; ; ) {
                readch();
                if (!Character.isDigit(peek)) break;
                x = x + Character.digit(peek, 10) / d;
                d *= 10;
            }
            return new Real(x, l, c);
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
                word = new Word(t, Tag.ID);
                wordTable.put(t, word);
            }
            word.setLine(l);
            word.setColumn(c);
            return word;
        }
        Token token = new Token(peek, l, c);
        peek = ' ';
        return token;
    }

    public boolean end() {
        try {
            return inputStream.available()==0;
        } catch (IOException e) {
            throw new Error(e.getMessage());
        }
    }
}
