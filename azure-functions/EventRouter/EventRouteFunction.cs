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
            // ��û ������ ���� �� �ֵ��� �غ�
            req.EnableBuffering(); // ��û ������ ���� �� ���� �� �ְ� ����

            var queryString = req.QueryString.Value;
            _logger.LogInformation("Query String: ");
            _logger.LogInformation(queryString);

            // ��û ������ ����
            string requestBody = await new StreamReader(req.Body, Encoding.UTF8).ReadToEndAsync();
            
            // ��û ���� �α׿� ���
            _logger.LogInformation("Request Body: ");
            _logger.LogInformation(requestBody);

            // ���� ��Ʈ�� ��ġ�� ó������ ���� (�ٸ� �̵��� ó���⿡�� ������ ���� �� �ֵ���)
            req.Body.Position = 0;

            // ���� ��ȯ
            return new OkObjectResult("Welcome to Azure Functions!");
        }
    }
}
