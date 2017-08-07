import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Collections;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.zip.GZIPInputStream;

/**
 * @author Pavel Stibal
 *
 * Solves all the logic.
 * There is an implemented handler for httpserver
 */
public class RequestHandler implements HttpHandler {

    private static final String POST_RESPONSE_MESSAGE = "Data is received\n";
    private static final String BAD_RESPONSE_MESSAGE = "\n";

    private Set<String> words;

    /***
     * Constructor for my handler, which is for my handler
     */
    public RequestHandler() {
        words = Collections.newSetFromMap(new ConcurrentHashMap<String, Boolean>());
    }

    /***
     * This method solves whether the request method is a get or a post.
     * For post, call the post methods.
     * At getu call the get method that calls the response
     * @param httpExchange Variable from the httpserver class
     * @throws IOException
     */
    @Override
    public void handle(HttpExchange httpExchange) throws IOException {
        TypeRequest type = TypeRequest.valueOf(httpExchange.getRequestMethod());

        switch (type){
            case GET:
                createResponseForGet(httpExchange);
                break;
            case POST:
                String text = convertReceiveDataToString(httpExchange.getRequestBody());
                splitTextAndFindUniqueWords(text);
                createResponseForPost(httpExchange);
                break;
            default:
                createBadResponse(httpExchange);
                break;
        }
    }

    /**
     * The method creates a response for the post.
     * Sends http_200 with sting, which is specified above is written
     * @param httpExchange Variable from the httpserver class
     * @throws IOException
     */
    private void createResponseForPost(HttpExchange httpExchange) throws IOException {
        httpExchange.sendResponseHeaders(200, POST_RESPONSE_MESSAGE.length());

        OutputStream outputStream = httpExchange.getResponseBody();
        outputStream.write(POST_RESPONSE_MESSAGE.getBytes());
        outputStream.close();
    }

    /**
     * This method returns the number of unique words as respone, which is http_200, and clears the set
     * @param httpExchange Variable from the httpserver class
     * @throws IOException
     */
    private void createResponseForGet(HttpExchange httpExchange) throws IOException {
        String response = String.valueOf(words.size() + "\n");
        words.clear();

        httpExchange.sendResponseHeaders(200, response.length());
        System.out.print(response);

        OutputStream outputStream = httpExchange.getResponseBody();
        outputStream.write(response.getBytes());
        outputStream.close();
    }

    /**
     * The method creates a response for the post.
     * Sends http_400 with sting, which is specified above is written
     * (If something went wrong by accident)
     * @param httpExchange Variable from the httpserver class
     * @throws IOException
     */
    private void createBadResponse(HttpExchange httpExchange) throws IOException {
        httpExchange.sendResponseHeaders(400, BAD_RESPONSE_MESSAGE.length());

        OutputStream outputStream = httpExchange.getResponseBody();
        outputStream.write(BAD_RESPONSE_MESSAGE.getBytes());
        outputStream.close();
    }

    /**
     * This method decompresses receive data to string
     * @param requestBody Body of request
     * @return Decompressed string
     * @throws IOException
     */
    private String convertReceiveDataToString(InputStream requestBody) throws IOException {
        ByteArrayOutputStream buffer = new ByteArrayOutputStream();

        int length;
        byte[] data = new byte[requestBody.toString().length()];
        GZIPInputStream gzis = new GZIPInputStream(requestBody);

        while ((length = gzis.read(data)) > 0) {
            buffer.write(data, 0, length);
        }

        return (new String(buffer.toByteArray()));
    }

    /**
     * The method will find a unique word that it adds to the set
     * @param text Decompressed string of received data
     */
    private void splitTextAndFindUniqueWords(String text){
        for (String word : text.split("\\s+")){
            if (!word.isEmpty() && !words.contains(word)){
                words.add(word);
            }
        }
    }
}
