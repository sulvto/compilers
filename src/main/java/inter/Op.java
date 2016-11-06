package inter;

import lexer.Token;
import symbols.Type;

/**
 * Created by sulvto on 16-11-6.
 */
public class Op extends Expr {
    public Op(Token token, Type p) {
        super(token, p);
    }

    @Override
    public Expr reduce() {
        Expr x = gen();
        Temp t = new Temp(type);
        emit(t.toString()+" = "+x.toString());
        return t;
    }
}
