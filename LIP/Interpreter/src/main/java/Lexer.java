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
    Hashtable<String, Integer> wordTable = new Hashtable<>();
    private InputStream inputStream;

    public Lexer(InputStream inputStream) {
        this.inputStream = inputStream;
        reserve("new", Type.NEW);
        reserve("if", Type.IF);
        reserve("else", Type.ELSE);
        reserve("def", Type.DEF);
        reserve("return", Type.RETURN);
        reserve("print", Type.PRINT);
        reserve("while", Type.WHILE);
        reserve("struct", Type.STRUCT);
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

    private Token newToken(int tag, String value) {
        return new Token(tag, value, line, column);
    }


    private Token newToken(int tag) {
        return newToken(tag, "");
    }


    public Token nextToken() {
        if (end()) return newToken(Type.EOF);

        for (; ; readch()) {
            if (peek == ' ' || peek == '\t') continue;
                //  \r or \n
                // \r\n ??
            else if (peek == '\n') {
                column = 1;
                line++;
                peek = ' ';
                return newToken(Type.NL);
            } else break;
        }

        if (end()) return newToken(Type.EOF);

        // #
        if (peek == '#') {
            do {
                readch();
            } while (peek != '\n');
            return nextToken();
        }

        int line = this.line;
        int column = this.column;

        switch (peek) {
            case '=':
                if (readch('=')) return newToken(Type.EQ);
                readch();
                return newToken(Type.ASSIGN);
            case '<':
                readch();
                return newToken(Type.LT);
            case '>':
                readch();
                return newToken(Type.GT);
            case '+':
                readch();
                return newToken(Type.ADD);
            case '-':
                readch();
                return newToken(Type.SUB);
            case '*':
                readch();
                return newToken(Type.MUL);
            case '/':
                readch();
                return newToken(Type.DIV);
            case '.':
                readch();
                return newToken(Type.DOT);
        }

        // "string"
        if (peek == '\"') {
            StringBuilder buf = new StringBuilder();
            do {
                readch();
                buf.append(peek);
            } while (peek != '\"');
            readch();
            return newToken(Type.STRING, buf.toString());
        }


        if (Character.isDigit(peek)) {
            StringBuilder v = new StringBuilder();
            do {
                v.append(peek);
                readch();
            } while (Character.isDigit(peek));

            if ('.' != peek) {
                return newToken(Type.INT, v.toString());
            }
            v.append('.');
            for (; ; ) {
                readch();
                if (!Character.isDigit(peek)) break;
                v.append(peek);
            }
            return newToken(Type.FLOAT, v.toString());
        }

        if (Character.isLetter(peek)) {
            StringBuilder buf = new StringBuilder();
            do {
                buf.append(peek);
                readch();
            } while (Character.isLetterOrDigit(peek));

            String t = buf.toString();
            Integer type = wordTable.get(t);
            if (type == null) {
                wordTable.put(t, Type.ID);
                return newToken(Type.ID, t);
            } else {
                return newToken(type, t);
            }

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
