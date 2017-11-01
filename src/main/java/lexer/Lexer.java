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
    Hashtable<String, Integer> wordTable = new Hashtable<>();
    private InputStream inputStream;

    public Lexer(InputStream inputStream) {
        this.inputStream = inputStream;

        reserve("true", Tag.TRUE);
        reserve("false", Tag.FALSE);
        reserve("void", Tag.VOID);
        reserve("char", Tag.CHAR);
        reserve("long", Tag.LONG);
        reserve("int", Tag.INT);
        reserve("float", Tag.FLOAT);
        reserve("bool", Tag.BOOL);
        reserve("union", Tag.UNION);
        reserve("enum", Tag.ENUM);
        reserve("static", Tag.STATIC);
        reserve("extern", Tag.EXTERN);
        reserve("const", Tag.CONST);
        reserve("if", Tag.IF);
        reserve("else", Tag.ELSE);
        reserve("switch", Tag.SWITCH);
        reserve("case", Tag.CASE);
        reserve("default", Tag.DEFAULT);
        reserve("while", Tag.WHILE);
        reserve("do", Tag.DO);
        reserve("for", Tag.FOR);
        reserve("return", Tag.RETURN);
        reserve("break", Tag.BREAK);
        reserve("continue", Tag.CONTINUE);
        reserve("goto", Tag.GOTO);
        reserve("typedef", Tag.TYPEDEF);
        reserve("struct", Tag.STRUCT);
        reserve("import", Tag.IMPORT);
        reserve("sizeof", Tag.SIZEOF);
    }


    private void reserve(String word, int tag) {
        wordTable.put(word, tag);
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
        if (end()) return new Token(Tag.EOF, line, column);

        for (; ; readch()) {
            if (peek == ' ' || peek == '\t') {
                if (end()) return new Token(Tag.EOF, line, column);
            } else if (peek == '\n') {
                column = 0;
                line++;
                if (end()) return new Token(Tag.EOF, line, column);
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
                    } else {
                        // TODO eof
                    }
                }
            } else {
                return new Token('/', line, column);
            }
        }

        int line = this.line;
        int column = this.column;

        switch (peek) {
            case '&':
                if (readch('&')) return new Token(Tag.AND, line, column, "&&");
                else return new Token('&', line, column);
            case '|':
                if (readch('|')) return new Token(Tag.OR, line, column, "||");
                else return new Token('|', line, column);
            case '!':
                if (readch('=')) return new Token(Tag.NE, line, column, "!=");
                else return new Token('!', line, column);
            case '=':
                if (readch('=')) return new Token(Tag.EQ, line, column, "==");
                else return new Token('=', line, column);
            case '>':
                if (readch('=')) return new Token(Tag.GE, line, column, ">=");
                else return new Token('<', line, column);
            case '<':
                if (readch('=')) return new Token(Tag.LE, line, column, "<=");
                else return new Token('<', line, column);
        }

        // "string"
        if (peek == '\"') {
            StringBuilder buf = new StringBuilder();
            do {
                readch();
                buf.append(peek);
            } while (peek != '\"');
            readch();
            return new Token(Tag.STRING, line, column, buf.toString());
        }

        // TODO Tag.CHARACTER

        if (Character.isDigit(peek)) {
            StringBuilder buf = new StringBuilder();
            do {
                buf.append(peek);
                readch();
            } while (Character.isDigit(peek));

            if ('.' != peek) {
                return new Token(Tag.NUM, line, column, buf.toString());
            }

            for (; ; ) {
                readch();
                if (!Character.isDigit(peek)) break;
                buf.append(peek);
            }
            return new Token(Tag.REAL, line, column, buf.toString());
        }

        if (Character.isLetter(peek)) {
            StringBuilder buf = new StringBuilder();
            do {
                buf.append(peek);
                readch();
            } while (Character.isLetterOrDigit(peek));

            String t = buf.toString();
            if (wordTable.get(t) != null) return new Token(wordTable.get(t), line, column, t);
            else return new Token(Tag.ID, line, column, t);
        }
        Token token = new Token(peek, line, column);
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
