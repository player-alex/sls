using System.Text;
using Microsoft.AspNetCore.Http;
using Microsoft.AspNetCore.Mvc;
using Microsoft.Azure.Functions.Worker;
using Microsoft.Extensions.Logging;

namespace EventRouter
{
    public class EventRouteFunction
    {
        private readonly ILogger<EventRouteFunction> _logger;

        public EventRouteFunction(ILogger<EventRouteFunction> logger)
        {
            _logger = logger;
        }

        [Function("route")]
        public async Task<IActionResult> Run(
            [HttpTrigger(AuthorizationLevel.Anonymous, "get", "post")] HttpRequest req)
        {
            // 요청 본문을 읽을 수 있도록 준비
            req.EnableBuffering(); // 요청 본문을 여러 번 읽을 수 있게 해줌

            var queryString = req.QueryString.Value;
            _logger.LogInformation("Query String: ");
            _logger.LogInformation(queryString);

            // 요청 본문을 읽음
            string requestBody = await new StreamReader(req.Body, Encoding.UTF8).ReadToEndAsync();
            
            // 요청 본문 로그에 출력
            _logger.LogInformation("Request Body: ");
            _logger.LogInformation(requestBody);

            // 본문 스트림 위치를 처음으로 리셋 (다른 미들웨어나 처리기에서 본문을 읽을 수 있도록)
            req.Body.Position = 0;

            // 응답 반환
            return new OkObjectResult("Welcome to Azure Functions!");
        }
    }
}
