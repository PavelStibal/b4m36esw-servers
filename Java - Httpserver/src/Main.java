import com.sun.net.httpserver.HttpServer;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

/**
 * @author Pavel Stibal
 *
 * I used this library https://docs.oracle.com/javase/8/docs/jre/api/net/httpserver/spec/com/sun/net/httpserver/HttpServer.html
 * I had to write my own handler for that class
 * In this class there is only initialization
 */
public class Main {

    private static final int PORT = 10101;
    private static final int BACKLOG = 0;
    private static final String POST_URL = "/esw/myserver/data";
    private static final String GET_URL = "/esw/myserver/count";

    public static void main(String[] args) throws IOException {
        HttpServer server = HttpServer.create(new InetSocketAddress(PORT), BACKLOG);
        RequestHandler handler = new RequestHandler();
        server.createContext(POST_URL, handler);
        server.createContext(GET_URL, handler);

        ExecutorService executorService = Executors.newFixedThreadPool(Runtime.getRuntime().availableProcessors());
        server.setExecutor(executorService);
        server.start();

        System.out.println("Running server");
    }
}
