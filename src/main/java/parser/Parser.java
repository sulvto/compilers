package parser;

import ast.*;
import entity.*;
import lexer.Lexer;
import lexer.Tag;
import lexer.Token;
import type.*;

import java.io.InputStream;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;
import java.util.Stack;

/**
 * Created by sulvto on 16-12-8.
 */
public class Parser {
    private Lexer input;
    private Stack<Integer> markerStack = new Stack<>();
    private List<Token> lookaheadList = new LinkedList<>();
    private List<Exception> speculateError = new ArrayList<>();
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
        return lookToken(i).type;
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
        } catch (Exception e) {
            success = false;
            speculateError.add(e);
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
        else
            error("Encountered \"" + Tag.string(lookahead()) + "\" . Was expecting one of:\"" + Tag.string(tag) + "\"");
    }

    public Parser(Lexer lex) {
        this.input = lex;
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
        match('(');
        Params params = params();
        match(')');
        BlockNode block = block();
        return new DefinedFunction(false, type, token.value, params, block);
    }

    DefinedVariable defvar() {
        TypeNode type = type();
        Token token = this.lookToken();
        match(Tag.ID);
        if (lookahead() == ';') {
            consume();
            return new DefinedVariable(false, type, token.value, null);
        } else if (lookahead() == '=') {
            consume();
            ExprNode expr = expr();
            match(';');
            return new DefinedVariable(false, type, token.value, expr);
        } else {
            error("defvar syntax error");
        }
        return null;
    }


    private Constant defconst() {
        match(Tag.CONST);
        TypeNode type = type();
        Token token = lookToken();
        match(Tag.ID);
        ExprNode expr = expr();
        return new Constant(false, type, token.value, expr);
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
                speculateError.forEach(e -> System.out.println(e.getMessage()));
                error("top_defs syntax error");

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
                return new IfNode(token, cond, s1, s2);
            case Tag.FOR:
                return forStmt();
            case Tag.WHILE:
                token = lookToken();
                match(Tag.WHILE);
                match('(');
                cond = bool();
                match(')');
                s1 = stmt();
                return new WhileNode(token, cond, s1);
            case Tag.DO:
                token = lookToken();
                match(Tag.DO);
                s1 = stmt();
                match(Tag.WHILE);
                match('(');
                cond = bool();
                match(')');
                match(';');
                return new DoWhileNode(token, s1, cond);
            case Tag.CONTINUE:
                token = lookToken();
                match(Tag.CONTINUE);
                match(';');
                return new ContinueNode(token);
            case Tag.BREAK:
                token = lookToken();
                match(Tag.BREAK);
                match(';');
                return new BreakNode(token);
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
            error("forInitStmt syntax error");
            return null;
        }
    }

    private StmtNode forStmt() {
        Token token = lookToken();
        match(Tag.FOR);
        match('(');
        Object init = null;
        if (lookahead() != ';') {
            init = forInitStmt();
        } else {
            consume();
        }
        ExprNode cond = null;
        if (lookahead() != ';') cond = bool();
        match(';');
        ExprNode incr = null;
        if (lookahead() != ')') {
            ExprNode left = term();
            if (lookahead() == '=') {
                Token op = lookToken();
                consume();
                incr = new AssignNode(left, op, bool());
            } else {
                // TODO []
            }
        }
        match(')');
        BlockNode body = block();
        if (init == null) {
            return new ForNode(token, cond, incr, body);
        }
        return (init instanceof ExprNode) ? new ForNode(token, (ExprNode) init, cond, incr, body) : new ForNode(token, (DefinedVariable) init, cond, incr, body);
    }

    private StmtNode returnStmt() {
        Token token = lookToken();
        match(Tag.RETURN);
        ExprNode expr = expr();
        match(';');
        return new ReturnNode(token, expr);
    }

    private StmtNode assign() {
        Token token = lookToken();

        StmtNode stmt;
        ExprNode left = term();
        if (lookahead() == '=') {
            Token op = lookToken();
            consume();
            stmt = new ExprStmtNode(token, new AssignNode(left, op, bool()));
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
            Token token = lookToken();
            consume();
            node = new LogicalOrNode(node, token, join());
        }
        return node;
    }

    private ExprNode join() {
        ExprNode node = equality();
        while (lookahead() == Tag.AND) {
            Token token = lookToken();
            consume();
            node = new LogicalAndNode(node, token, equality());
        }
        return node;
    }

    private ExprNode equality() {
        ExprNode node = rel();
        while (lookahead() == Tag.EQ || lookahead() == Tag.NE) {
            Token token = lookToken();
            consume();
            node = new BinaryOpNode(node, token, equality());
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
                node = new BinaryOpNode(node, token, expr());
            default:
                return node;
        }
    }

    private ExprNode expr() {
        ExprNode node = term();
        while (lookahead() == '+' || lookahead() == '-') {
            Token token = lookToken();
            consume();
            node = new BinaryOpNode(node, token, term());
        }
        return node;
    }

    private ExprNode term() {
        ExprNode node = unary();
        while (lookahead() == '*' || lookahead() == '/' || lookahead() == '%') {
            Token token = lookToken();
            consume();
            node = new BinaryOpNode(node, token, unary());
        }
        return node;
    }

    private ExprNode unary() {
        switch (lookahead()) {
            case '+':
            case '-':
            case '~':
            case '!':
                Token token = lookToken();
                consume();
                return new UnaryOpNode(token, term());
            default:
                return factor();
        }
    }

    private ExprNode factor() {
        Token token;
        switch (lookahead()) {
            case '(':
                consume();
                ExprNode node = bool();
                match(')');
                return node;
//            case Tag.REAL:
            // TODO
            case Tag.NUM:

                token = lookToken();
                consume();
                // TODO int long
                return new IntegerLiteralNode(token, IntegerTypeRef.intRef(), Integer.valueOf(token.value));
            // TODO
//            case Tag.TRUE:
//            case Tag.TRUE:
            case Tag.CHARACTER:
                token = lookToken();
                consume();
                return new IntegerLiteralNode(token, IntegerTypeRef.charRef(), token.value.charAt(0));
            case Tag.STRING:
                token = lookToken();
                consume();
                return new StringLiteralNode(token, new PointerTypeRef(IntegerTypeRef.charRef()), token.value);
            case Tag.ID:
                token = lookToken();
                consume();
                return new VariableNode(token, token.value);
            default:
                error("factor syntax error --> " + Tag.string(lookahead()));
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
        return new BlockNode(token, definedVariables, stmts);
    }

    private Parameter param() {
        TypeNode type = type();
        Token id = lookToken();
        match(Tag.ID);
        return new Parameter(true, type, id.value);
    }

    private Params params() {
        Token token = lookToken();
        List<Parameter> params = new ArrayList<>();
        while (speculate.apply(this::param)) {
            params.add(param());
            if (lookahead() == ',') consume();
            else break;
        }
        return new Params(token, params);
    }


    //   TODO typeref_base

    TypeNode type() {
        return new TypeNode(typeRef());
    }

    TypeRef typeRef() {
        TypeRef ref = typeRefBase();
        // []  [x] *  (params)

        if (lookahead() == '[') {
            match('[');
            Token length = null;
            if (lookahead() == Tag.ID) {
                length = lookToken();
                consume();
            }
            match(']');
            return new ArrayTypeRef(ref, length);
        } else if (lookahead() == '*') {
            match('*');
            return new PointerTypeRef(ref);
        } else if (lookahead() == '(') {
            match('(');
            match(')');
            return new FunctionTypeRef(ref);
        }

        return ref;
    }

    TypeRef typeRefBase() {
        Token name = null;
        Token token = lookToken();
        switch (lookahead()) {
            case Tag.VOID:
                consume();
                return new VoidTypeRef(token);
            case Tag.INT:
                consume();
                return IntegerTypeRef.intRef(token);
            case Tag.LONG:
                consume();
                return IntegerTypeRef.longRef(token);
            case Tag.CHAR:
                consume();
                return IntegerTypeRef.charRef(token);
            case Tag.SHORT:
                consume();
                return IntegerTypeRef.shortRef(token);
            case Tag.STRUCT:
                consume();
                name = lookToken();
                match(Tag.ID);
                return new StructTypeRef(token, name);
            case Tag.UNION:
                consume();
                name = lookToken();
                match(Tag.ID);
                return new UnionTypeRef(token, name);
            default:
                // TODO UserTypeRef
                return null;
        }
    }


    TypedefNode typedef() {
        Token token = lookToken();
        match(Tag.TYPEDEF);
        TypeRef typeRef = typeRef();
        Token word = lookToken();
        match(Tag.ID);
        TypedefNode typedefNode = new TypedefNode(token, typeRef, word.value);
        return typedefNode;
    }

    private Location location(Token token) {
        return new Location(token.line, token.column);
    }

    private void error(String message) {
        Token token = lookToken();
        throw new RuntimeException("[ERROR] line " + token.line + ", column " + token.column + ".\n" + message);
    }
}
