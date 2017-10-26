import entity.Constant;
import entity.DefinedFunction;
import entity.DefinedVariable;
import entity.Params;
import lexer.*;

import java.io.InputStream;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;
import java.util.Stack;

/**
 * Created by sulvto on 17-10-26.
 */
public class Parser {
    private Lexer input;
    private Stack<Integer> markerStack = new Stack<>();
    private List<Token> lookaheadList = new LinkedList<>();

    private int p = 0;


    private boolean isSpeculating() {
        return !markerStack.isEmpty();
    }

    private void sync(int i) {
        if (p + i - 1 > (lookaheadList.size() - 1)) {
            int n = p + i - 1 - (lookaheadList.size() - 1);
            fill(n);
        }
    }

    private void fill(int n) {
        for (int i = 1; i <= n; i++) {
            lookaheadList.add(input.nextToken());
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
        return lookToken(i).tag;
    }

    private int lookahead() {
        return lookahead(1);
    }

    private int mark() {
        markerStack.push(p);
        return p;
    }

    private void release() {
        int marker = markerStack.pop();
        seek(marker);
    }

    private void seek(int index) {
        p = index;
    }

    interface Speculate {
        void apply();
    }

    java.util.function.Function<Speculate, Boolean> speculate = supplier -> {
        boolean success = true;
        mark();
        try {
            supplier.apply();
        } catch (Error e) {
            success = false;
        }
        release();
        return success;
    };

    void consume() {
        p++;
        if (p == lookaheadList.size() && !isSpeculating()) {
            p = 0;
            lookaheadList.clear();
        }
        sync(1);
    }

    void match(int... tags) {
        for (int i = 0; i < tags.length; i++) {
            if (tags[i] != lookahead()) error("TODO");
        }
    }

    void match(int tag) {
        if (tag == lookahead()) consume();
        else error("Encountered \"" + (char) lookahead() + "\" . Was expecting one of:\"" + (char) tag + "\"");
    }

    public Parser(Lexer lex) {
        this.input = lex;
        consume();
    }

    public Parser(InputStream inputStream) {
        this(new Lexer(inputStream));
    }

    AST compilationUnit() {
        // TODO import
        Declarations declarations = top_defs();
        return new AST(declarations);
    }

    DefinedFunction defun() {
        TypeNode type = type();
        Token token = this.lookToken();
        match(Tag.ID);
        Word word = (Word) token;
        match('(');
        Params params = params();
        match(')');
        BlockNode block = block();
        return new DefinedFunction(false, type, word.lexeme, params, block);
    }

    DefinedVariable defvar() {
        TypeNode type = type();
        Token token = this.lookToken();
        match(Tag.ID);
        Word word = (Word) token;
        if (lookahead() == ';') {
            consume();
            return new DefinedVariable(false, type, word.lexeme, null);
        } else if (lookahead() == '=') {
            consume();
            ExprNode expr = expr();
            match(';');
            return new DefinedVariable(false, type, word.lexeme, expr);
        } else {
            error("syntax error");
        }
        return null;
    }


    private Constant defconst() {
        match(Tag.CONST);
        TypeNode type = type();
        Token token = lookToken();
        match(Tag.ID);
        ExprNode expr = expr();
        return new Constant(false, type, ((Word) token).lexeme, expr);
    }

    private StructNode defstruct() {
        match(Tag.STRUCT);
        // TODO
        return null;
    }


    Declarations top_defs() {
        Declarations declarations = new Declarations();
        while (true) {
            if (speculate.apply(this::defun)) {
                declarations.addDefun(defun());
            } else if (speculate.apply(this::defvar)) {
                declarations.addDefvar(defvar());
            } else if (speculate.apply(this::defconst)) {
                declarations.addConstant(defconst());
            } else if (speculate.apply(this::defstruct)) {
                declarations.addDefstruct(defstruct());
            } else if (speculate.apply(this::typedef)) {
                declarations.addTypedef(typedef());
            } else if (speculate.apply(this::defunion)) {
                declarations.addDefunion(defunion());
            } else if (speculate.apply(() -> match(Tag.EOF))) {
                return declarations;
            } else {
                error("syntax error");
            }
        }
    }

    private UnionNode defunion() {
        match(Tag.UNION);
        // TODO
        return null;
    }


    List<StmtNode> stmts() {
        if (lookahead() == '}') {
            return new ArrayList<>();
        }

        List<StmtNode> result = new ArrayList<>();
        for (; lookahead() != '}'; ) {
            StmtNode stmt = stmt();
            if (stmt == null) {
                return result;
            }
            result.add(stmt);
        }
        return result;
    }

    private StmtNode stmt() {
        Token token;
        ExprNode cond;
        StmtNode s1 = null, s2 = null;
        switch (lookahead()) {
            case ';':
                consume();
                // TODO null
                return null;
            case Tag.IF:
                token = lookToken();
                match(Tag.IF);
                match('(');
                cond = bool();
                match(')');
                s1 = stmt();
                if (lookahead() == Tag.ELSE) {
                    match(Tag.ELSE);
                    s2 = stmt();
                }
                return new IfNode(location(token), cond, s1, s2);
            case Tag.FOR:
                return forStmt();
            case Tag.WHILE:
                token = lookToken();
                match(Tag.WHILE);
                match('(');
                cond = bool();
                match(')');
                s1 = stmt();
                return new WhileNode(location(token), cond, s1);
            case Tag.DO:
                token = lookToken();
                match(Tag.DO);
                s1 = stmt();
                match(Tag.WHILE);
                match('(');
                cond = bool();
                match(')');
                match(';');
                return new DoWhileNode(location(token), s1, cond);
            case Tag.CONTINUE:
                token = lookToken();
                match(Tag.CONTINUE);
                match(';');
                return new ContinueNode(location(token));
            case Tag.BREAK:
                token = lookToken();
                match(Tag.BREAK);
                match(';');
                return new BreakNode(location(token));
            case Tag.RETURN:
                return returnStmt();
            case '{':
                return block();
            default:
                return assign();
        }
    }

    private Object forInitStmt() {
        if (speculate.apply(this::assign)) {
            return assign();
        } else if (speculate.apply(this::defvar)) {
            return defvar();
        } else {
            error("syntax error");
            return null;
        }
    }

    private StmtNode forStmt() {
        Token token = lookToken();
        match(Tag.FOR);
        match('(');
        Object init = forInitStmt();
        ExprNode cond = null;
        if (lookahead() != ';') cond = bool();
        match(';');
        ExprNode incr = null;
        if (lookahead() != ')') {
            ExprNode left = term();
            if (lookahead() == '=') {
                consume();
                incr = new AssignNode(left, bool());
            } else {
                // TODO []
            }
        }
        match(')');
        BlockNode body = block();
        if (init == null) {
            return new ForNode(location(token), cond, incr, body);
        }
        return (init instanceof ExprNode) ? new ForNode(location(token), (ExprNode) init, cond, incr, body) : new ForNode(location(token), (DefinedVariable) init, cond, incr, body);
    }

    private StmtNode returnStmt() {
        Token token = lookToken();
        match(Tag.RETURN);
        ExprNode expr = expr();
        match(';');
        return new ReturnNode(location(token), expr);
    }

    private StmtNode assign() {
        Token token = lookToken();

        StmtNode stmt;
        ExprNode left = term();
        if (lookahead() == '=') {
            consume();
            stmt = new ExprStmtNode(location(token), new AssignNode(left, bool()));
        } else {
            // TODO []
            stmt = null;
        }
        match(';');
        return stmt;
    }

    private ExprNode bool() {
        ExprNode node = join();
        while (lookahead() == Tag.OR) {
            consume();
            node = new LogicalOrNode(node, join());
        }
        return node;
    }

    private ExprNode join() {
        ExprNode node = equality();
        while (lookahead() == Tag.AND) {
            consume();
            node = new LogicalAndNode(node, equality());
        }
        return node;
    }

    private ExprNode equality() {
        ExprNode node = rel();
        while (lookahead() == Tag.EQ || lookahead() == Tag.NE) {
            Token token = lookToken();
            consume();
            node = new BinaryOpNode(node, "" + (char) token.tag, equality());
        }
        return node;
    }

    private ExprNode rel() {
        ExprNode node = expr();
        switch (lookahead()) {
            case '<':
            case Tag.LE:
            case Tag.GE:
            case '>':
                Token token = lookToken();
                consume();
                node = new BinaryOpNode(node, "" + (char) token.tag, expr());
            default:
                return node;
        }
    }

    private ExprNode expr() {
        ExprNode node = term();
        while (lookahead() == '+' || lookahead() == '-') {
            Token token = lookToken();
            consume();
            node = new BinaryOpNode(node, "" + (char) token.tag, term());
        }
        return node;
    }

    private ExprNode term() {
        ExprNode node = unary();
        while (lookahead() == '*' || lookahead() == '/' || lookahead() == '%') {
            Token token = lookToken();
            consume();
            node = new BinaryOpNode(node, "" + (char) token.tag, unary());
        }
        return node;
    }

    private ExprNode unary() {
        switch (lookahead()) {
            case '-':
            case '!':
                Token token = lookToken();
                consume();
                return new UnaryOpNode("" + (char) token.tag, unary());
            default:
                return factor();
        }
    }

    private ExprNode factor() {
        Token token;
        Word word;
        Num num;
        switch (lookahead()) {
            case '(':
                consume();
                ExprNode node = bool();
                match(')');
                return node;
//            case Tag.REAL:
            // TODO
            case Tag.NUM:

                num = (Num) lookToken();
                consume();
                // TODO int long
                return new IntegerLiteralNode(location(num), IntegerTypeRef.intRef(), num.value);
            // TODO
//            case Tag.TRUE:
//            case Tag.TRUE:
            case Tag.CHARACTER:
                num = (Num) lookToken();
                consume();
                return new IntegerLiteralNode(location(num), IntegerTypeRef.charRef(), num.value);
            case Tag.STRING:
                word = (Word) lookToken();
                consume();
                return new StringLiteralNode(location(word), new PointerTypeRef(IntegerTypeRef.charRef()), word.lexeme);
            case Tag.ID:
                word = (Word) lookToken();
                consume();
                return new VariableNode(location(word), word.lexeme);
            default:
                error("syntax error --> " + (char) lookahead());
                return null;
        }

    }


    private BlockNode block() {
        Token token = lookToken();
        match('{');
        List<DefinedVariable> definedVariables = new ArrayList<>();

        while (speculate.apply(this::defvar)) {
            definedVariables.add(defvar());
        }

        List<StmtNode> stmts = stmts();
        match('}');
        return new BlockNode(location(token), definedVariables, stmts);
    }


    private Params params() {
        return null;
    }


    //   TODO typeref_base

    TypeNode type() {
        return new TypeNode(typeRef());
    }

    TypeRef typeRef() {
        TypeRef ref = typeRefBase();
        // []  [x] *  (params)
        return ref;
    }

    TypeRef typeRefBase() {
        Word name = null;
        Token token = lookToken();
        switch (lookahead()) {
            case Tag.VOID:
                consume();
                return new VoidTypeRef(location(token));
            case Tag.INT:
                consume();
                return IntegerTypeRef.intRef(location(token));
            case Tag.LONG:
                consume();
                return IntegerTypeRef.longRef(location(token));
            case Tag.CHAR:
                consume();
                return IntegerTypeRef.charRef(location(token));
            case Tag.SHORT:
                consume();
                return IntegerTypeRef.shortRef(location(token));
            case Tag.STRUCT:
                consume();
                name = (Word) lookToken();
                match(Tag.ID);
                return new StructTypeRef(location(token), name.lexeme);
            case Tag.UNION:
                consume();
                name = (Word) lookToken();
                match(Tag.ID);
                return new UnionTypeRef(location(token), name.lexeme);
            default:
                // TODO UserTypeRef
                return null;
        }
    }


    TypedefNode typedef() {
        Token token = lookToken();
        match(Tag.TYPEDEF);
        TypeRef typeRef = typeRef();
        Word word = (Word) lookToken();
        match(Tag.ID);
        TypedefNode typedefNode = new TypedefNode(location(token), typeRef, word.lexeme);
        return typedefNode;
    }

    private Location location(Token token) {
        return new Location(token.line, token.column);
    }

    private void error(String message) {
        Token token = lookToken();
        throw new Error("[ERROR] line " + token.line + ", column " + token.column + ".\n" + message);
    }
}
