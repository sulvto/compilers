package parser;

import ast.*;
import entity.*;
import lexer.*;
import util.Speculate;

import java.io.IOException;
import java.io.InputStream;
import java.util.*;

/**
 * Created by sulvto on 16-12-8.
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

    Declarations top_defs() {
        Declarations declarations = new Declarations();
        switch (lookahead()) {
            case Tag.TYPEDEF:
                declarations.addTypedef(typedef());
                break;
            case Tag.BASIC:
                TypeNode type = type();
                Token token = this.lookToken();
                match(Tag.ID);
                Word word = (Word) token;
                if (lookahead() == ';') {
                    declarations.addDefvar(new DefinedVariable(false, type, word.lexeme, null));
                    match(';');
                    break;
                } else if (lookahead() == '=') {
                    ExprNode expr = expr();
                    declarations.addDefvar(new DefinedVariable(false, type, word.lexeme, expr));
                    match(';');
                    break;
                } else if (lookahead() == '(') {
                    match('(');
                    Params params = params();
                    match(')');
                    BlockNode block = block();
                    declarations.addDefun(new DefinedFunction(false, type, word.lexeme, params, block));
                    break;
                } else {
                    error("syntax error");
                }
            case Tag.STRUCT:
                declarations.addDefstruct(struct());
                break;
            case Tag.CONST:
                declarations.addConstant(constant());
                break;
            case Tag.UNION:
                declarations.addDefunion(union());
                break;
            default:
                return declarations;
        }
        return declarations;
    }


    List<DefinedVariable> defVar() {
        List<DefinedVariable> result = new ArrayList<>();

        while (lookahead() == Tag.BASIC) {
            TypeNode type = type();
            Token token = this.lookToken();
            match(Tag.ID);
            Word word = (Word) token;
            if (lookahead() == ';') {
                consume();
                result.add(new DefinedVariable(false, type, word.lexeme, null));
            } else if (lookahead() == '=') {
                consume();
                ExprNode expr = expr();
                result.add(new DefinedVariable(false, type, word.lexeme, expr));
                match(';');
            } else {
                error("syntax error");
            }

        }
        return result;
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
        ExprNode cond;
        StmtNode s1 = null, s2 = null;
        switch (lookahead()) {
            case ';':
                consume();
                // TODO null
                return null;
            case Tag.IF:
                match(Tag.IF);
                match('(');
                cond = bool();
                match(')');
                s1 = stmt();
                if (lookahead() == Tag.ELSE) {
                    match(Tag.ELSE);
                    s2 = stmt();
                }
                return new IfNode(cond, s1, s2);
            case Tag.FOR:
                return forStmt();
            case Tag.WHILE:
                match(Tag.WHILE);
                match('(');
                cond = bool();
                match(')');
                s1 = stmt();
                return new WhileNode(cond, s1);
            case Tag.DO:
                match(Tag.DO);
                s1 = stmt();
                match(Tag.WHILE);
                match('(');
                cond = bool();
                match(')');
                match(';');
                return new DoWhileNode(s1, cond);
            case Tag.CONTINUE:
                match(Tag.CONTINUE);
                match(';');
                return new ContinueNode();
            case Tag.BREAK:
                match(Tag.BREAK);
                match(';');
                return new BreakNode();
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
        } else if (speculate.apply(this::defVar)) {
            List<DefinedVariable> definedVariables = defVar();
            if (definedVariables.size() > 0) {
                return definedVariables;
            } else {
                match(';');
                return null;
            }
        } else {
            error("syntax error");
            return null;
        }
    }

    private StmtNode forStmt() {
        match(Tag.FOR);
        match('(');
        Object init = forInitStmt();
        ExprNode cond = null;
        if (lookahead() != ';') cond = bool();
        match(';');
        ExprNode incr = null;
        if (lookahead() != ')') incr = expr();
        match(')');
        BlockNode body = block();
        return (init instanceof ExprNode) ? new ForNode((ExprNode) init, cond, incr, body) : new ForNode((List<DefinedVariable>) init, cond, incr, body);
    }

    private StmtNode returnStmt() {
        match(Tag.RETURN);
        ExprNode expr = expr();
        match(';');
        return new ReturnNode(expr);
    }

    private StmtNode assign() {
        StmtNode stmt;
        Token t = lookToken();
        match(Tag.ID);
        if (lookahead() == '=') {
            consume();
            stmt = new ExprStmtNode(new AssignNode(((Word) t).lexeme, bool()));
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
        switch (lookahead()) {
            case '(':
                consume();
                ExprNode node = bool();
                match(')');
                return node;
//            case Tag.REAL:
            // TODO
            case Tag.NUM:
                Num num = (Num) lookToken();
                consume();
                return new IntegerLiteralNode(num.value);
            // TODO
//            case Tag.TRUE:
//            case Tag.TRUE:
//            case Tag.CHARACTER:
//            case Tag.STRING:
            case Tag.ID:
                Word word = (Word) lookToken();
                consume();
                return new VariableNode(word.lexeme);
            default:
                error("syntax error --> " + (char) lookahead());
                return null;
        }

    }

    private Constant constant() {
        match(Tag.CONST);
        TypeNode type = type();
        Token token = lookToken();
        match(Tag.ID);
        ExprNode expr = expr();
        return new Constant(false, type, ((Word) token).lexeme, expr);
    }

    private UnionNode union() {
        // TODO
        return null;
    }

    private StructNode struct() {
        return null;
    }

    private BlockNode block() {
        match('{');
        List<DefinedVariable> definedVariables = defVar();
        List<StmtNode> stmts = stmts();
        match('}');
        return new BlockNode(definedVariables, stmts);
    }


    private Params params() {
        return null;
    }

    TypeNode type() {
        Type type = (Type) this.lookToken();
        match(Tag.BASIC);
        if (lookahead() == '[') {
            // TODO array
        }
        return new TypeNode(type.name());
    }


    TypedefNode typedef() {
        match(Tag.TYPEDEF);
        match(Tag.BASIC);
        // TODO  array or strtus
        TypedefNode typedefNode = new TypedefNode();
        return typedefNode;
    }

    private void error(String message) {
        throw new Error("[ERROR] line " + input.line + ", column " + input.column + ".\n" + message);
    }
}
