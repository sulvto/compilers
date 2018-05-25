package assembler;

import java.io.InputStream;
import java.util.LinkedList;
import java.util.List;

import static assembler.Type.*;

/**
 * Created by sulvto on 17-10-29.
 */
public class AssemblerParser {

    private AssemblerLexer input;
    private List<Token> lookaheadList = new LinkedList<>();

    private int p = 0;

    private void sync(int i) {
        if (p + i - 1 > (lookaheadList.size() - 1)) {
            int n = p + i - 1 - (lookaheadList.size() - 1);
            fill(n);
        }
    }

    private void fill(int n) {
        for (int i = 1; i <= n; i++) {
            Token token = input.nextToken();
            lookaheadList.add(token);
        }
    }

    private Token lookToken(int i) {
        sync(i);
        return lookaheadList.get(p + i - 1);
    }

    private Token lookToken() {
        return lookToken(1);
    }

    private int lookahead(int i) {
        return lookToken(i).type;
    }

    private int lookahead() {
        return lookahead(1);
    }

    void consume() {
        p++;
        if (p == lookaheadList.size()) {
            p = 0;
            lookaheadList.clear();
        }
        sync(1);
    }

    void match(int tag) {
        if (tag == lookahead()) consume();
        else error("Encountered \"" + (char) lookahead() + "\" . Was expecting one of:\"" + (char) tag + "\"");
    }

    public AssemblerParser(AssemblerLexer lex) {
        this.input = lex;
    }

    public AssemblerParser(InputStream inputStream) {
        this(new AssemblerLexer(inputStream));
    }

    public void program() {
        while (lookahead() == NEWLINE) consume();
        if (lookahead(2) == GLOBALS) globals();

        while (true) {
            if (lookahead() == NEWLINE) consume();
            else if (lookahead() == EOF) break;
            else if (lookahead() == '.') functionDeclaration();
            else if (lookahead() == ID) {
                // instr or label
                if (lookahead(2) == ':') label();
                else instr();
            } else error("error program");
        }
        checkForUnresolvedReferences();
    }

    private void globals() {
        match('.');
        match(GLOBALS);
        Token dataSize = lookToken();
        match(INT);
        match(NEWLINE);
        defineDataSize(Integer.parseInt(dataSize.text));
    }

    private void functionDeclaration() {
        match('.');
        match(DEF);
        Token name = lookToken();
        match(ID);
        match(':');
        match(ARGS);
        match('=');
        Token args = lookToken();
        match(INT);
        match(',');
        match(LOCALS);
        match('=');
        Token locals = lookToken();
        match(INT);
        match(NEWLINE);
        defineFunction(name, Integer.parseInt(args.text), Integer.parseInt(locals.text));
    }

    private void instr() {
        Token instr = lookToken();
        match(ID);
        if (NL_EOF()) {
            consume();
            gen(instr);
            return;
        }
        Token o1 = operand();
        if (NL_EOF()) {
            consume();
            gen(instr, o1);
            return;
        }
        match(',');
        Token o2 = operand();
        if (NL_EOF()) {
            consume();
            gen(instr, o1, o2);
            return;
        }
        match(',');
        Token o3 = operand();
        match(ID);
        MATCH_NL_EOF();

        gen(instr, o1, o2, o3);
    }

    private Token operand() {
        Token token = lookToken();
        switch (lookahead()) {
            case INT:
            case CHAR:
            case STRING:
            case FLOAT:
            case REG:
            case FUNC:
                consume();
                return token;
            default:
                break;
        }
        if (lookahead() == ID) {
            consume();
            // FUNC
            if (lookahead() == '(') {
                match('(');
                match(')');
                return new Token(FUNC, token.text, token.line, token.column);
            }
            return token;
        }
        error("error operand");
        return null;
    }

    private void label() {
        Token label = lookToken();
        match(ID);
        match(':');
        defineLabel(label);
    }

    private void MATCH_NL_EOF() {
        if (lookahead() == NEWLINE) match(NEWLINE);
        else match(EOF);
    }

    private boolean NL_EOF() {
        int lookahead = lookahead();
        return lookahead == NEWLINE || lookahead == EOF;
    }

    private void error(String message) {
        Token token = lookToken();
        throw new Error("[ERROR] line " + token.line + ", column " + token.column + ".\n" + message);
    }

    protected void gen(Token instrToken) {
        throw new UnsupportedOperationException();
    }

    protected void gen(Token instrToken, Token oToken1) {
        throw new UnsupportedOperationException();
    }

    protected void gen(Token instrToken, Token oToken1, Token oToken2) {
        throw new UnsupportedOperationException();
    }

    protected void gen(Token instrToken, Token oToken1, Token oToken2, Token oToken3) {
        throw new UnsupportedOperationException();
    }

    protected void defineLabel(Token idToken) {
        throw new UnsupportedOperationException();
    }

    protected void defineFunction(Token idToken, int nargs, int nlocals) {
        throw new UnsupportedOperationException();
    }

    protected void defineDataSize(int dataSize) {
        throw new UnsupportedOperationException();
    }

    protected void checkForUnresolvedReferences() {
        throw new UnsupportedOperationException();
    }

}

