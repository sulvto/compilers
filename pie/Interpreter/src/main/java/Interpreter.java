import java.io.IOException;
import java.io.InputStream;
import java.util.Stack;


/**
 * Created by sulvto on 17-10-26.
 */
public class Interpreter {
    public InterpreterListener listener = new InterpreterListener() {
        @Override
        public void info(String msg) {
            System.out.println(msg);
        }

        @Override
        public void error(String msg) {
            System.out.println(msg);
        }

        @Override
        public void error(String msg, Exception e) {
            error(msg);
            e.printStackTrace();
        }

        @Override
        public void error(String msg, Token t) {
            error("line " + t.line + ": " + msg);
        }
    };

    GlobalScope globalScope;
    MemorySpace globals = new MemorySpace("globals");
    MemorySpace currentSpace = globals;
    Stack<FunctionSpace> stack = new Stack<>();
    private static final ReturnValue sharedReturnValue = new ReturnValue();

    public void interp(InputStream input) throws IOException {
        globalScope = new GlobalScope();
        PieParser parser = new PieParser(input, globalScope);
        PieAST root = parser.program();
        System.out.println(root.toStringTree());
        block(root);
    }

    private Object exec(PieAST ast) {
        try {
            switch (ast.getType()) {
                case Type.BLOCK:
                    block(ast);
                    break;
                case Type.ASSIGN:
                    assign(ast);
                    break;
                case Type.RETURN:
                    ret(ast);
                    break;
                case Type.PRINT:
                    print(ast);
                    break;
                case Type.IF:
                    ifstat(ast);
                    break;
                case Type.WHILE:
                    whileloop(ast);
                    break;
                case Type.CALL:
                    return call(ast);
                case Type.NEW:
                    return instance(ast);
                case Type.ADD:
                    return add(ast);
                case Type.SUB:
                    return op(ast);
                case Type.MUL:
                    return op(ast);
                case Type.EQ:
                    return eq(ast);
                case Type.LT:
                    return lt(ast);
                case Type.GT:
                    return gt(ast);
                case Type.INT:
                    return Integer.parseInt(ast.getText());
                case Type.CHAR:
                    return ast.getText().charAt(1);
                case Type.STRING:
                    String text = ast.getText();
                    return text.substring(0, text.length() - 1);
                case Type.FLOAT:
                    return Float.parseFloat(ast.getText());
                case Type.DOT:
                case Type.ID:
                    return load(ast);
                default:
                    throw new UnsupportedOperationException("Node " + ast.getText() + "<" + ast.getType() + "> not handled");
            }
        } catch (Exception e) {
            listener.error("problem executing " + ast.toStringTree(), e);
        }
        return null;
    }

    private Object instance(PieAST ast) {
        PieAST structNameNode = ast.getChild(0);
        StructSymbol s = (StructSymbol) structNameNode.scope.resolve(structNameNode.getText());
        return new StructInstance(s);
    }

    private Object load(PieAST ast) {
        if (ast.getType() == Type.DOT) return fieldLoad(ast);
        MemorySpace space = getSpaceWithSymbol(ast.getText());
        if (space != null) return space.get(ast.getText());
        listener.error("no such variable  " + ast.getText(), ast.token);
        return null;
    }

    private Object fieldLoad(PieAST ast) {
        PieAST expr = ast.getChild(0);
        PieAST b = ast.getChild(1);
        String id = b.getText();
        StructInstance struct = (StructInstance) load(expr);
        if (struct.def.resolveMember(id) == null) {
            listener.error(struct.name + " has no " + id + " filed", b.token);
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
            float x = ((Number) a).floatValue();
            float y = ((Number) b).floatValue();
            switch (ast.getType()) {
                case Type.ADD:
                    return x + y;
                case Type.SUB:
                    return x - y;
                case Type.MUL:
                    return x * y;
                case Type.DIV:
                    return x / y;
            }
        }
        if (a instanceof Number || b instanceof Number) {
            int x = ((Number) a).intValue();
            int y = ((Number) b).intValue();
            switch (ast.getType()) {
                case Type.ADD:
                    return x + y;
                case Type.SUB:
                    return x - y;
                case Type.MUL:
                    return x * y;
                case Type.DIV:
                    return x / y;
            }
        }

        return 0;
    }

    private boolean lt(PieAST ast) {
        Object a = exec(ast.getChild(0));
        Object b = exec(ast.getChild(1));
        if (a instanceof Number && b instanceof Number) {
            return ((Number) a).floatValue() < ((Number) b).floatValue();
        }
        return false;
    }

    private boolean gt(PieAST ast) {
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

    private void whileloop(PieAST ast) {
        PieAST condStart = ast.getChild(0);
        PieAST codeStart = ast.getChild(1);
        while ((Boolean) exec(condStart)) {
            exec(codeStart);
        }
    }

    private void ifstat(PieAST ast) {
        PieAST condStart = ast.getChild(0);
        PieAST codeStart = ast.getChild(1);
        PieAST elseCodeStart = null;
        if (ast.getChildCount() == 3) elseCodeStart = ast.getChild(2);
        Boolean c = (Boolean) exec(condStart);
        if (c) exec(codeStart);
        else if (elseCodeStart != null) exec(elseCodeStart);
    }

    private void ret(PieAST ast) {
        sharedReturnValue.value = exec(ast.getChild(0));
        throw sharedReturnValue;
    }


    private void print(PieAST ast) {
        PieAST expr = ast.getChild(0);
        System.out.println(exec(expr));
    }


    private Object call(PieAST ast) {
        String name = ast.getChild(0).getText();
        FunctionSymbol funSymbol = (FunctionSymbol) ast.scope.resolve(name);
        if (funSymbol == null) {
            listener.error("no such function " + name, ast.token);
            return null;
        }
        FunctionSpace funSpace = new FunctionSpace(funSymbol);
        MemorySpace saveSpace = currentSpace;
        currentSpace = funSpace;
        int argCount = ast.getChildCount() - 1;

        if (funSymbol.formalArgs == null && argCount > 0 ||
                funSymbol.formalArgs != null && funSymbol.formalArgs.size() != argCount) {
            listener.error("function " + funSymbol.name + " argument list mismatch");
            return null;
        }
        int i = 0;
        for (Symbol argS : funSymbol.formalArgs.values()) {
            VariableSymbol arg = (VariableSymbol) argS;
            PieAST ihhArg = ast.getChild(i + 1);
            Object argValue = exec(ihhArg);
            funSpace.put(arg.name, argValue);
            i++;
        }
        Object result = null;
        stack.push(funSpace);
        try {
            exec(funSymbol.blockAST);
        } catch (ReturnValue rv) {
            result = rv.value;
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
        if (lhs.getType() == Type.DOT) {
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
        PieAST o = lhs.getChild(0);
        PieAST f = lhs.getChild(1);
        String fieldName = f.getText();
        Object a = load(o);
        if (!(a instanceof StructInstance)) {
            String leftPart = o.getText();

            listener.error(leftPart + "is not a struct ", o.token);
            return;
        }


        StructInstance struct = (StructInstance) a;

        if (struct.def.resolveMember(fieldName) == null) {
            listener.error("can`t assign: " + struct.name + " has no " + fieldName + " field", f.token);
            return;
        }

        struct.put(fieldName, value);
    }

}
