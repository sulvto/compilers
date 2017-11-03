package ir;

import asm.Type;
import entity.ConstantEntity;

/**
 * Created by sulvto on 17-11-4.
 */
public class Str extends Expr {
    private ConstantEntity entity;

    public Str(Type type, ConstantEntity entity) {
        super(type);
        this.entity = entity;
    }

    public ConstantEntity entity() {
        return entity;
    }

    @Override
    public boolean isConstant() {
        return true;
    }

    @Override
    public <S, E> E accept(IRVisitor<S, E> visitor) {
        return visitor.visit(this);
    }

    @Override
    protected void duDump(Dumper dumper) {
        dumper.printMember("entity", entity);
    }
}
