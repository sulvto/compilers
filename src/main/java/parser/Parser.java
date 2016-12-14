package parser;

import ast.*;
import entity.Constant;
import entity.DefinedFunction;
import entity.DefinedVariable;
import entity.Params;
import lexer.*;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

/**
 * Created by sulvto on 16-12-8.
 */
public class Parser {
    public Lexer lex;
    public Token look;

    public Parser(Lexer lex) throws IOException {
        this.lex = lex;
        move();
    }

    void move() throws IOException {
        look = lex.scan();
    }

    AST compilationUnit() throws IOException {
        // TODO
        Declarations declarations = top_defs();
        return new AST(declarations);
    }

    Declarations top_defs() throws IOException {
        Declarations declarations = new Declarations();
        switch (look.tag) {
            case Tag.TYPEDEF:
                declarations.addTypedef(typedef());
                break;
            case Tag.BASIC:
                TypeNode type = type();
                Token token = this.look;
                match(Tag.ID);
                Word word = (Word) token;
                if (look.tag == ';') {
                    declarations.addDefvar(new DefinedVariable(false, type, word.lexeme, null));
                    match(';');
                    break;
                } else if (look.tag == '=') {
                    ExprNode expr = expr();
                    declarations.addDefvar(new DefinedVariable(false, type, word.lexeme, expr));
                    match(';');
                    break;
                } else if (look.tag == '(') {
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

    List<DefinedVariable> defVar() throws IOException {
        List<DefinedVariable> result = new ArrayList<>();

        while (look.tag == Tag.BASIC) {
            TypeNode type = type();
            Token token = this.look;
            match(Tag.ID);
            Word word = (Word) token;
            if (look.tag == ';') {
                result.add(new DefinedVariable(false, type, word.lexeme, null));
            } else if (look.tag == '=') {
                ExprNode expr = expr();
                result.add(new DefinedVariable(false, type, word.lexeme, expr));
            } else {
                error("syntax error");
            }
            match(';');
        }
        return result;
    }


    List<StmtNode> stmts() {
        if (look.tag == '}') {
            return new ArrayList<>();
        }

        List<StmtNode> result = new ArrayList<>();
        for(;;) {
            StmtNode stmt = stmt();
            if (stmt == null) {
                return result;
            }
            result.add(stmt);
        }
    }

    private StmtNode stmt() throws IOException {
        ExprNode cond;
        StmtNode s1 = null, s2 = null;
        switch (look.tag) {
            case ';':
                move();
                // TODO null
                return null;
            case Tag.IF:
                match(Tag.IF);
                match('(');
                cond = bool();
                match(')');
                s1 = stmt();
                if (look.tag == Tag.ELSE){
                    match(Tag.ELSE);
                    s2 = stmt();
                }
                return new IfNode(cond, s1, s2);
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
            case '{':
                return block();
            default:
                return exprStmt();
        }
    }

    private StmtNode exprStmt() {
        ExprNode expr = expr();
        return new ExprStmtNode(expr);
    }

    private StmtNode assign() throws IOException {
        StmtNode stmt;
        Token t = look;
        match(Tag.ID);
        if (look.tag == '=') {
            move();
            stmt = new AssignNode();
        }
        return null;
    }

    private ExprNode bool() {

        return null;
    }

    private Constant constant() throws IOException {
        match(Tag.CONST);
        TypeNode type = type();
        Token token = look;
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

    private BlockNode block() throws IOException {
        match('{');
        List<DefinedVariable> definedVariables = defVar();
        List<StmtNode> stmts = stmts();
        match('}');
        return new BlockNode(definedVariables, stmts);
    }

    private Params params() {
        return null;
    }

    private ExprNode expr() {
        // assign

        return null;
    }

    TypeNode type() throws IOException {
        Type type = (Type) this.look;
        match(Tag.BASIC);
        if (look.tag == '[') {
            // TODO array
        }
        return new TypeNode(type.name());
    }

    TypedefNode typedef() throws IOException {
        match(Tag.TYPEDEF);
        match(Tag.BASIC);
        // TODO  array or strtus
        TypedefNode typedefNode = new TypedefNode();
        return typedefNode;
    }

    void match(int tag) throws IOException {
        if (tag == look.tag) move();
        else error("Encountered \"" + (char) look.tag + "\" . Was expecting one of:\"" + (char) tag + "\"");
    }

    private void error(String message) {
        throw new Error("[ERROR] line " + lex.line + ", column " + lex.column + ".\n" + message);
    }
}
