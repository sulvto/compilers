package interpreters;

import entity.Entity;

import java.util.ArrayList;
import java.util.List;

/**
 * Created by sulvto on 17-1-8.
 */
public abstract class Scope {

    abstract public boolean isToplevel();

    abstract public ToplevelScope toplevel();

    abstract public Scope parent();

    abstract public Entity getEntity(String name);

}
