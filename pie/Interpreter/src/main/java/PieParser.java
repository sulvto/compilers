import java.io.InputStream;
import java.util.LinkedList;
import java.util.List;
import java.util.Stack;

/**
 * Created by sulvto on 17-10-26.
 */
public class PieParser {

    private Scope currentScope;
    private Lexer input;
    private Stack<Integer> markerStack = new Stack<>();
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

    public PieParser(Lexer lex, Scope scope) {
        this.input = lex;
        this.currentScope = scope;
    }

    public PieParser(InputStream inputStream, Scope scope) {
        this(new Lexer(inputStream), scope);
    }

    public PieAST program() {
        PieAST root = new PieAST(Type.BLOCK);
        while (true) {
            if (lookahead() == Type.DEF) {
                functionDefinition();
            } else if (lookahead() == Type.EOF) {
                break;
            } else {
                PieAST statrment = statrment();
                if (statrment != null) root.addChild(statrment);
            }
        }
        return root;
    }

    private void functionDefinition() {
        match(Type.DEF);
        Token id = lookToken();
        match(Type.ID);
        FunctionSymbol functionSymbol = new FunctionSymbol(id.text, currentScope);
        currentScope.define(functionSymbol);
        currentScope = functionSymbol;

        match('(');
        // Variable
        if (lookahead() == Type.ID) {
            while (true) {
                vardef();
                if (lookahead() == ',') consume();
                else break;
            }
        }
        match(')');
        // block
        currentScope = new LocalScope(functionSymbol);

        functionSymbol.blockAST = slist();
        currentScope = currentScope.getEnclosingScope().getEnclosingScope();
    }

    private PieAST slist() {
        PieAST ast = new PieAST(Type.BLOCK);
        if (lookahead() == ':') {
            match(':');
            match(Type.NL);
            while (true) {

                PieAST statrment = statrment();
                if (statrment != null) ast.addChild(statrment);
                if (lookahead() == Type.DOT) {
                    match(Type.DOT);
                    NL_OR_EOF();
                    break;
                }
            }
        } else {
            PieAST statrment = statrment();
            if (statrment != null) ast.addChild(statrment);
        }
        return ast;
    }

    private PieAST statrment() {
        switch (lookahead()) {
            case Type.STRUCT:
                structDefinition();
                return null;
            case Type.RETURN:
                return returnExpr();
            case Type.PRINT:
                return printExpr();
            case Type.IF:
                return ifExpr();
            case Type.WHILE:
                return whileExpr();
            case Type.ID:
                if (lookahead(2) == '(') return call();
                else return assign();
            case Type.NL:
                consume();
                return null;
            default:
                error("error statrment");
        }
        return null;
    }

    private PieAST assign() {
        PieAST qid = qid();
        PieAST ast = new PieAST(lookToken());
        ast.addChild(qid);
        match(Type.ASSIGN);
        ast.addChild(expr());
        return ast;
    }

    private PieAST call() {
        PieAST ast = new PieAST(new Token(Type.CALL));
        ast.scope = currentScope;
        ast.addChild(new PieAST(lookToken()));
        match(Type.ID);
        match('(');
        if (lookahead() == ')') {
            consume();
            return ast;
        }

        ast.addChild(expr());
        while (lookahead() == ',') {
            consume();
            ast.addChild(expr());
        }
        match(')');
        return ast;
    }

    private PieAST ifExpr() {
        match(Type.IF);
        PieAST ast = new PieAST(Type.IF);
        ast.addChild(expr());
        ast.addChild(slist());
        if (lookahead() == Type.ELSE) {
            consume();
            ast.addChild(slist());
        }
        return ast;
    }

    private PieAST printExpr() {
        match(Type.PRINT);
        PieAST ast = new PieAST(Type.PRINT);
        ast.addChild(expr());
        NL_OR_EOF();
        return ast;
    }

    private PieAST returnExpr() {
        match(Type.RETURN);
        PieAST ast = new PieAST(Type.RETURN);
        ast.addChild(expr());
        NL_OR_EOF();
        return ast;
    }

    private PieAST expr() {
        PieAST left = addExpr();
        while (lookahead() == Type.LT || lookahead() == Type.GT) {
            PieAST oldLeft = left;
            left = new PieAST(lookToken());
            consume();
            left.addChild(oldLeft);
            left.addChild(addExpr());
        }
        return left;
    }

    private PieAST addExpr() {
        PieAST left = muleExpr();
        while (lookahead() == Type.ADD || lookahead() == Type.SUB) {
            PieAST oldLeft = left;
            left = new PieAST(lookToken());
            consume();
            left.addChild(oldLeft);
            left.addChild(muleExpr());
        }
        return left;
    }

    private PieAST muleExpr() {
        PieAST left = atom();
        while (lookahead() == Type.MUL || lookahead() == Type.DIV) {
            PieAST oldLeft = left;
            left = new PieAST(lookToken());
            consume();
            left.addChild(oldLeft);
            left.addChild(atom());
        }
        return left;
    }

    private PieAST atom() {
        switch (lookahead()) {
            case Type.INT:
            case Type.FLOAT:
            case Type.CHAR:
            case Type.STRING:
                Token token = lookToken();
                consume();
                return new PieAST(token);
            case Type.NEW:
                return instance();
            case Type.ID:
                if (lookahead(2) == '(') return call();
                else return qid();
            case '(':
                match('(');
                PieAST expr = expr();
                match(')');
                return expr;
            default:
                error("err atom");
        }
        return null;
    }

    private PieAST qid() {
        PieAST ast = new PieAST(lookToken());
        match(Type.ID);
        // if or while ?
        if (lookahead() == Type.DOT) {
            PieAST left = ast;
            ast = new PieAST(lookahead());
            consume();
            ast.addChild(left);
            ast.addChild(new PieAST(lookToken()));
            match(Type.ID);
        }
        return ast;
    }

    private PieAST instance() {
        PieAST ast = new PieAST(lookToken());
        match(Type.NEW);
        PieAST child = new PieAST(lookToken());
        child.scope = currentScope;
        ast.addChild(child);
        match(Type.ID);
        return ast;
    }

    private PieAST whileExpr() {
        PieAST ast = new PieAST(lookToken());
        match(Type.WHILE);
        ast.addChild(expr());
        ast.addChild(slist());
        return ast;
    }

    private void vardef() {
        Token id = lookToken();
        match(Type.ID);
        currentScope.define(new VariableSymbol(id.text));
    }

    private void structDefinition() {
        match(Type.STRUCT);
        Token name = lookToken();
        match(Type.ID);
        match('{');
        StructSymbol struct = new StructSymbol(name.text, currentScope);
        currentScope.define(struct);
        currentScope = struct;

        if (lookahead() == Type.ID) {
            while (true) {
                vardef();
                if (lookahead() == ',') consume();
                else break;
            }
        }

        match('}');


        currentScope = currentScope.getEnclosingScope();
    }

    private void NL_OR_EOF() {
        if (lookahead() == Type.NL) match(Type.NL);
        else match(Type.EOF);
    }

    private void error(String message) {
        Token token = lookToken();
        throw new Error("[ERROR] line " + token.line + ", column " + token.column + ".\n" + message);
    }
}
