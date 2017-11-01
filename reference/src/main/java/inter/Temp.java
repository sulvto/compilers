package inter;

import symbols.Type;

/**
 * Created by sulvto on 16-11-6.
 */
public class Temp extends Expr {
    static int count = 0;
    int number = 0;

    public Temp(Type p) {
        super(Word.temp,p);
        number = ++count;
    }

    @Override
    public String toString() {
        return "t" + number;
    }
}
