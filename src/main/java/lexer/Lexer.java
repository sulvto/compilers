package lexer;

import java.io.IOException;
import java.io.InputStream;
import java.util.Hashtable;

/**
 * Created by sulvto on 16-12-8.
 */
public class Lexer {
    public static int line = 1;
    public static int column = 0;

    char peek = ' ';
    Hashtable<String, Word> wordTable = new Hashtable<>();
    private InputStream inputStream;

    public Lexer(InputStream inputStream) {
        this.inputStream = inputStream;
        reserve(Word.AND);
        reserve(Word.BREAK);
        reserve(Word.CASE);
        reserve(Word.CONST);
        reserve(Word.CONTINUE);
        reserve(Word.CHAR);
        reserve(Word.DEFAULT);
        reserve(Word.DO);
        reserve(Word.ELSE);
        reserve(Word.ENUM);
        reserve(Word.EQ);
        reserve(Word.EXTERN);
        reserve(Word.FALSE);
        reserve(Word.FOR);
        reserve(Word.GE);
        reserve(Word.IF);
        reserve(Word.IMPORT);
        reserve(Word.LE);
        reserve(Word.NE);
        reserve(Word.OR);
        reserve(Word.RETURN);
        reserve(Word.GOTO);
        reserve(Word.SIZEOF);
        reserve(Word.STATIC);
        reserve(Word.SWITCH);
        reserve(Word.TRUE);
        reserve(Word.TYPEDEF);
        reserve(Word.UNION);
        reserve(Word.VOID);
        reserve(Word.WHILE);
        reserve(Type.INT);
        reserve(Type.SHORT);
        reserve(Type.CHAR);
        reserve(Type.FLOAT);
        reserve(Type.BOOL);
    }


    private void reserve(Word word) {
        wordTable.put(word.lexeme, word);
    }


    private void readch() throws IOException {
        peek = (char) inputStream.read();
        column++;
    }

    private boolean readch(char c) throws IOException {
        readch();
        if (peek == c) {
            peek = ' ';
            return true;
        } else {
            return false;
        }
    }

    public Token scan() throws IOException {
        for (; ; readch()) {
            if (peek == ' ' || peek == '\t') ;
            else if (peek == '\n') line++;
            else break;
        }

        switch (peek) {
            case '&':
                if (readch('&')) return Word.AND;
                else return new Token('&');
            case '|':
                if (readch('|')) return Word.OR;
                else return new Token('|');
            case '!':
                if (readch('=')) return Word.OR;
                else return new Token('!');
            case '=':
                if (readch('=')) return Word.NE;
                else return new Token('=');
            case '>':
                if (readch('=')) return Word.GE;
                else return new Token('<');
            case '<':
                if (readch('=')) return Word.LE;
                else return new Token('<');

        }

        if (Character.isDigit(peek)) {
            int v = 0;
            do {
                v = 10 * v + Character.digit(peek, 10);
            } while (Character.isDigit(peek));

            if ('.' != peek) {
                return new Num(v);
            }

            float x = v;
            float d = 10;
            for (; ; ) {
                readch();
                if (!Character.isDigit(peek)) break;
                x = x + Character.digit(peek, 10) / d;
                d *= 10;
            }
            return new Real(x);
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
            return word;
        }

        Token token = new Token(peek);
        peek = ' ';
        return token;
    }
}
