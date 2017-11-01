package entity;

import exception.SemanticException;

import java.util.List;
import java.util.Map;

/**
 * Created by sulvto on 17-11-1.
 */
public class ToplevelScope extends Scope {
    // symbol table
    // function
    protected Map<String, Entity> entities;
    protected List<DefinedVariable> staticLocalVariables;

    /**
     * declare variable or function
     *
     * @param entity
     */
    public void declareEntity(Entity entity) throws SemanticException {
        Entity e = entities.get(entity.getName());
        if (e != null) {
            throw new SemanticException("duplicated declaration: " + entity.getName() + ": " + e.location() + " and " + entity.location());
        }
        entities.put(entity.getName(), entity);

    }

    public void defineEntity(Entity entity) {

    }

    @Override
    public Entity get(String name) throws SemanticException {
        Entity entity = entities.get(name);
        if (entity == null) {
            throw new SemanticException("unresolved reference: " + name);
        }
        return entity;
    }
}
