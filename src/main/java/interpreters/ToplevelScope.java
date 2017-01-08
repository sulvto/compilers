package interpreters;

import entity.Entity;

/**
 * Created by sulvto on 17-1-8.
 */
public class ToplevelScope extends Scope{
    @Override
    public boolean isToplevel() {
        return true;
    }

    @Override
    public ToplevelScope toplevel() {
        return this;
    }

    @Override
    public Scope parent() {
        return null;
    }

    @Override
    public Entity getEntity(String name) {
        // TODO
        return null;
    }
}
