package ir;

import asm.Type;
import entity.Entity;

/**
 * Created by sulvto on 17-11-4.
 */
public class Var extends Expr {
    private Entity entity;

    public Var(Type type, Entity entity) {
        super(type);
        this.entity = entity;
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
