//import jdk.nashorn.internal.parser.Lexer;
//import jdk.nashorn.internal.parser.Token;
//
//import java.util.List;
//import java.util.Map;
//
///**
// * stat : list EOF
// *      | list '=' list
// *      ;
// * Created by sulvto on 16-12-17.
// */
//public class BacktrackParser {
//    private Lexer lexer;
//    List<Integer> markers;
//    List<Token> lookahead;
//    int p = 0;
//    public static final int FAILED = -1;
//
//    public Token LT(int i) {
//        sync(i);
//        return lookahead.get(p + i - 1);
//    }
//
//    private void sync(int i) {
//        if (p + i - 1 > (lookahead.size() - 1)) {
//            int n = p + i - 1 - (lookahead.size() - 1);
//            fill(n);
//        }
//    }
//
//    private void fill(int n) {
//        for (int i=1;i<n;i++){lookahead.add(input.nextToken())}
//    }
//
//    public void consume() {
//        p++;
//        if(p==lookahead.size()&&!isSpeculating())
//    }
//
//
//    public boolean alreadyParsedRule(Map<Integer,Integer> memoization){
//        Integer memoI = memoization.get(index());
//        if(memoI==null) return false;
//        int memo = memoI.intValue();
//        System.out.print("parsed list brfore at index "+index()+"; skip ahead to token index "+memo+": "+lookahead.get(memo).text);
//        if (memo == FAILED) throw new PreviousParseFailedException();
//            seek(memo);
//            return true;
//    }
//
//    private void seek(int index) {
//        p = index;
//    }
//
//    public void memoize(Map<Integer ,Integer> memoization,int startTokenIndex,boolean failed) {
//        int stopTokenIndex = failed ? FAILED : index();
//        memoization.put(startTokenIndex, stopTokenIndex);
//    }
//
//    public void list() {
//        boolean failed = false;
//        int startTokenIndex = index();
//        if(isSpeculating() && alreadyParsedRule(list_memo)) return;
//        try {
//            _list();
//        } catch (RecognitionException exception) {
//            failed = true;
//            throw exception;
//        }finally {
//            if(isSpeculating()) memoize(list_memo, startTokenIndex, failed);
//        }
//
//    }
//    public void _list() {
//        System.out.print("parser list rule at token index:" + index());
//        match(BacktrackLexer.LBARACK);
//        elements();
//        match(BacktrackLexer.RBARACK);
//    }
//
//    private int index() {
//        return p;
//    }
//
//    public boolean isSpeculating() {
//        return !markers.isEmpty();
//    }
//
//}
