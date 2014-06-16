using System.Collections.Generic;
using System.Threading;


public class MessageManager
{
    public delegate void EventHandler ();

    private static Mutex msgMutex = new Mutex ();
    private static int messages = 0;
    private static Dictionary<int, EventHandler> msgHandlers = 
        new Dictionary<int, EventHandler> ();

    public static bool SendMessage (int msg)
    {
        return dispatchMessage (msg);
    }

    private static bool dispatchMessage (int msg)
    {
        if (msgHandlers.ContainsKey (msg)) {
            msgHandlers [msg] ();
            return true;
        }
        return false;
    }

    public static bool RegisterHander (int msg, EventHandler handler)
    {
        if (msgHandlers.ContainsKey (msg)) {
            msgHandlers [msg] += handler;
            return true;
        }
        return false;
    }

    public static int CreatMessage ()
    {
        int rstMsg;
        msgMutex.WaitOne ();

        EventHandler tmpHandler = null;
        msgHandlers [messages] = tmpHandler;
        rstMsg = messages;
        messages++;

        msgMutex.ReleaseMutex ();
        return rstMsg;
    }

    public static bool FindMessage (int msg)
    {
        return msgHandlers.ContainsKey (msg);
    }

}
