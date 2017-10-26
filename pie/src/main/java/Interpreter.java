import javax.xml.bind.JAXBElement;

import java.util.Stack;


/**
 * Created by sulvto on 17-10-26.
 */
public class Interpreter {
    GlobalScope globalScope;
    MemorySpace globals = new MemorySpace("globals");
    MemorySpace currentSpace = globals;
    Stack<FunctionSpace> stack = new Stack<>();
    private static final ReturnValue sharedReturnValue = new ReturnValue();

    public Object exec(PieAST ast) {
        try {


            switch (ast.getType()) {
                case PieParser.BLOCK:
                    block(ast);
                    break;
                case PieParser.ASSIGN:
                    assign(ast);
                    break;
                case PieParser.RETURN:
                    ret(ast);
                    break;
                case PieParser.PRINT:
                    print(ast);
                    break;
                case PieParser.IF:
                    ifstat(ast);
                    break;
                case PieParser.WHILE:
                    whileloop(ast);
                    break;
                case PieParser.CALL:
                    return call(ast);
                case PieParser.NEW:
                    return instance(ast);
                case PieParser.ADD:
                    return add(ast);
                case PieParser.SUB:
                    return op(ast);
                case PieParser.MUL:
                    return op(ast);
                case PieParser.EQ:
                    return eq(ast);
                case PieParser.LT:
                    return lt(ast);
                case PieParser.INT:
                    return Integer.parseInt(ast.getText());
                case PieParser.CHAR:
                    return ast.getText().charAt(1);
                case PieParser.FLOAT:
                    return Float.parseFloat(ast.getText());
                case PieParser.DOT:
                case PieParser.ID:
                    return load(ast);
                default:
                    throw new UnsupportedOperationException("Node " + ast.getText() + "<" + ast.getType() + "> not handled");
            }
        } catch (Exception e) {
            listener.error("problem executing " + ast.toStringTree(), e);
        }
        return null;
    }

    private Object load(PieAST ast) {
        if (ast.getType() != PieParser.DOT) return fieldLoad(ast);
        MemorySpace space = getSpaceWithSymbol(ast.getText());
        if (space != null) space.get(ast.getText());
        listener.error("no such variable  " + ast.getText(), ast.token);
        return null;
    }

    private Object fieldLoad(PieAST ast) {
        PieAST expr = ast.getChild(0);
        PieAST b = ast.getChild(1);
        String id = b.getText();
        StructInstance struct = (StructInstance) load(expr);
        if (sturct.def.resolveMember(id) == null) {
            listener.error(struct.name+" has no "+id+" filed",b.token);
            return null;
        }
        return struct.get(id);
    }

    private Object add(PieAST ast) {
        Object a = exec(ast.getChild(0));
        Object b = exec(ast.getChild(1));
        if (a instanceof String || b instanceof String) {
            return a.toString() + b.toString();
        }
        return op(ast);
    }

    private Object op(PieAST ast) {
        Object a = exec(ast.getChild(0));
        Object b = exec(ast.getChild(1));

        if (a instanceof Float || b instanceof Float) {
            float x = (Float) a;
            float y = (Float) b;
            switch (ast.getType()) {
                case PieParser.ADD:
                    return x + y;
                case PieParser.SUB:
                    return x - y;
                case PieParser.MUL:
                    return x * y;
            }
        }
        if (a instanceof Number || b instanceof Number) {
            int x = ((Number) a).intValue();
            int y = ((Number) b).intValue();
            switch (ast.getType()) {
                case PieParser.ADD:
                    return x + y;
                case PieParser.SUB:
                    return x - y;
                case PieParser.MUL:
                    return x * y;
            }
        }

        return 0;
    }

    private boolean lt(PieAST ast) {
        Object a = exec(ast.getChild(0));
        Object b = exec(ast.getChild(1));
        if (a instanceof Number && b instanceof Number) {
            return ((Number) a).floatValue() > ((Number) b).floatValue();
        }
        return false;
    }

    private Object eq(PieAST ast) {
        Object a = exec(ast.getChild(0));
        Object b = exec(ast.getChild(1));
        return a.equals(b);
    }

    public void whileloop(PieAST ast) {
        PieAST condStart = ast.getChild(0);
        PieAST codeStart = ast.getChild(1);
        Boolean c = (Boolean) exec(condStart);
        while (c) {
            exec(codeStart);
            c = (Boolean) exec(condStart);
        }
    }

    public void ifstat(PieAST ast) {
        PieAST condStart = ast.getChild(0);
        PieAST codeStart = ast.getChild(1);
        PieAST elseCodeStart = null;
        if (ast.getChildCount() == 3) elseCodeStart = ast.getChild(2);
        Boolean c = (Boolean) exec(condStart);
        if (c) exec(codeStart);
        else if (elseCodeStart != null) exec(elseCodeStart);
    }

    public void ret(PieAST ast) {
        sharedReturnValue.value = exec(ast.getChild(0));
        throw sharedReturnValue;
    }


    private void print(PieAST ast) {
        PieAST expr = (PieAST) exec(ast.getChild(0));
        System.out.println(exec(expr));
    }


    private Object call(PieAST ast) {
        String name = ast.getChild(0).getText();
        FunctionSpace funSpace = ast.scope.resolve(name);
        if (funSpace == null) {
            listener.error("no such function " + name, ast.token);
            return null;
        }
        FunctionSpace fSpace = new FunctionSpace(funSpace);
        MemorySpace saveSpace = currentSpace;
        currentSpace = fSpace;
        int argCount = ast.getChildCount() - 1;

        if (fSpace.formalArgs != null && argCount > 0 ||
                fSpace.formalArgs != null && fSpace.formalArgs.size() != argCount) {
            listener.error("function " + name + " argument list mismatch");
            return null;
        }
        int i = 0;
        for (Symbol argS : fSpace.formalArgs.values()) {
            VariableSymbol arg = (VariableSymbol) argS;
            PieAST ihhArg = ast.getChild(i + 1);
            Object argValue = exec(ihhArg);
            fSpace.put(arg.name, argValue);
            i++;
        }
        Object result = null;
        stack.push(fSpace);
        try {
            exec(funSpace.blockAST);
        } catch (ReturnValue rv) {
            result = rv;
        }
        stack.pop();
        currentSpace = saveSpace;
        return result;
    }

    private void block(PieAST ast) {
        ast.getChildren().forEach(this::exec);
    }

    private void assign(PieAST ast) {
        PieAST lhs = ast.getChild(0);
        PieAST expr = ast.getChild(1);
        Object value = exec(expr);
        if (lhs.getType() == PieParser.DOT) {
            fieldAssign(lhs, value);
            return;
        }


        MemorySpace space = getSpaceWithSymbol(lhs.getText());
        if (space == null) space = currentSpace;
        space.put(lhs.getText(), value);
    }

    private MemorySpace getSpaceWithSymbol(String id) {
        if (stack.size() > 0 && stack.peek().get(id) != null) {
            return stack.peek();
        }
        if (globals.get(id) != null) return globals;
        return null;
    }


    private void fieldAssign(PieAST lhs, Object value) {
    }
}
