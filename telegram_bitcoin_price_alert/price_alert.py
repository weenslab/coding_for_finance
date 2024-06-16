import datetime
from pytz import timezone
import requests
import telegram_send
import asyncio

async def main():
        now_utc = datetime.datetime.now(timezone('UTC'))
        now_wib = now_utc.astimezone(timezone('Asia/Jakarta'))
        ct = now_wib.strftime("%d %b %Y, %H:%M")

        coin_list = ['BTCUSDT','ETHUSDT']
        api_url = "https://api.binance.us/api/v3/ticker/price?symbol="
        responses = []
        for c in coin_list:
                #print(api_url + c)
                responses.append(requests.get(api_url + c))

        msg = "My Price Alert\n" + ct + "\n"
        for r in responses:
                msg = msg + r.json().get("symbol") + ": " + str(float(r.json().get("price"))) + "\n"
        #print(msg)

        await telegram_send.send(messages=[msg])

asyncio.run(main())
