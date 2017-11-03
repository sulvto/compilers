package ir;

import asm.Type;

/**
 * Created by sulvto on 17-11-3.
 */
public abstract class Expr implements Dumpable {
    final Type type;

    Expr(Type type) {
        this.type = type;
    }

    public Type type() {
        return type;
    }

    public boolean isVar() {
        return false;
    }


    public boolean isAddr() {
        return false;
    }


    public boolean isConstant() {
        return false;
    }


    public boolean asmValue() {
        throw new UnsupportedOperationException();
    }

//    public Operand address() {
//        throw new UnsupportedOperationException();
//    }


    abstract public <S, E> E accept(IRVisitor<S, E> visitor);


    @Override
    public void dump(Dumper dumper) {
        dumper.printClass(this);
        dumper.printMember("type", type);
        duDump(dumper);
    }

    protected abstract void duDump(Dumper dumper);
}
