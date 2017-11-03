package ir;

import asm.Type;
import entity.Entity;

/**
 * Created by sulvto on 17-11-4.
 */
public class Addr extends Expr {
    private Entity entity;

    Addr(Type type, Entity entity) {
        super(type);
        this.entity = entity;
    }

    @Override
    public boolean isAddr() {
        return true;
    }

    public Entity entity() {
        return entity;
    }

    @Override
    public <S, E> E accept(IRVisitor<S, E> visitor) {
        return visitor.visit(this);
    }


    @Override
    protected void duDump(Dumper dumper) {
        dumper.printMember("entity", entity.name());
    }
}
